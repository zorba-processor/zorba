/*
 * Copyright 2006-2008 The FLWOR Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "debugger/debugger_server.h"

#include <memory>
#include <fstream>

#include <zorba/zorba.h>
#include <zorba/util/path.h>

#include "debugger/utils.h"
#include "debugger/socket.h"
#include "debugger/message_factory.h"

#include "context/static_context.h"

#include "runtime/fncontext/FnContextImpl.h"
#include "runtime/core/item_iterator.h"
#include "runtime/api/runtimecb.h"
#include "runtime/api/plan_iterator_wrapper.h"
#include "runtime/util/iterator_impl.h"

#include "api/serialization/serializer.h"
#include "api/unmarshaller.h"

#include "zorbatypes/numconversions.h"

#include "common/shared_types.h"

#ifdef WIN32
#include <windows.h>
#define sleep(s) Sleep(s*1000)
#endif

namespace zorba{

ZORBA_THREAD_RETURN runtimeThread( void *aDebugger )
{
  ZorbaDebugger * lDebugger = (ZorbaDebugger *)aDebugger;
  //We wait for theRuntimeThread to be allocated
  while( lDebugger->theRuntimeThread == 0 )
  {
	  sleep(1);
  }
  lDebugger->runQuery();
  return 0;
}

ZorbaDebugger::~ZorbaDebugger()
{
  delete theRequestServerSocket;
  delete theEventSocket;
}

void ZorbaDebugger::start( XQueryImpl *aQuery,
                           std::ostream& aOutStream,
                           const Zorba_SerializerOptions_t& aSerOptions,
                           unsigned short aRequestPortno,
                           unsigned short aEventPortno)
{
  std::auto_ptr<TCPSocket> lSock;
  //Set the query and serialization options
  theQuery = aQuery;
  theOutputStream = &aOutStream;
  theSerOptions = &aSerOptions;
  //Run the server 
  theRequestServerSocket = new TCPServerSocket( aRequestPortno );
  theEventPortno = aEventPortno;
  
  //Wait for a client to connect
  lSock.reset(theRequestServerSocket->accept());
  assert( lSock.get() != 0 );
#ifndef NDEBUG
  std::clog << "[Server Thread] Client connected" << std::endl;
#endif
  //Try to connect to the event server 3 times
  for ( unsigned int i = 0; i < 3 && ! theEventSocket; i++ )
  {
    try
    {
      //Wait one second before trying to reconnect
      sleep(1);
      //Connect the client to the event server
      theEventSocket = new TCPSocket( lSock->getForeignAddress(), theEventPortno );
      break;
    } catch ( DebuggerSocketException &e )  {
      if ( i < 2 ) continue;
      std::cerr << "[Server Thread] Couldn't connect to the debugger server event: " << std::endl;
      std::cerr << "[Server Thread] " <<  e.what() << std::endl;
      return;
    }
  }
#ifndef NDEBUG
  std::clog << "[Server Thread] Connected to the event server" << std::endl;
#endif

  //Perform handshake
  handshake( lSock.get() );
  
  //Until the query execution has ended
  while ( theStatus != QUERY_TERMINATED )
  {
    handleTcpClient( lSock.get() );
  }
#ifndef NDEBUG
  std::clog << "[Server Thread] server quited" << std::endl;
#endif
  delete theRuntimeThread;
#ifndef NDEBUG
  std::clog << "[Server Thread] runtime thread quited" << std::endl;
#endif 
}

void ZorbaDebugger::setStatus( ExecutionStatus Status,
                                   SuspensionCause aCause ){

  if ( theStatus == QUERY_SUSPENDED && Status == QUERY_RUNNING )
  {
    theStatus = QUERY_RESUMED;
  } else {
	  theStatus = Status;
  }

  switch ( theStatus )
  {
    case QUERY_RUNNING:
      startedEvent();
      break;
    case QUERY_SUSPENDED:
      suspendedEvent( theLocation, aCause );
      break;
    case QUERY_TERMINATED:
      terminatedEvent();
      break;
    case QUERY_RESUMED:
      theStatus = QUERY_RUNNING;
      resumedEvent();
      break;
  }
}

void ZorbaDebugger::sendEvent( AbstractCommandMessage * aMessage )
{
    Length length;
    ZorbaArrayAutoPointer<Byte> lMessage(aMessage->serialize(length));
    try
    {
#ifndef NDEBUG
      std::clog << "[Server Thread] send an event: ";
      switch ( aMessage->getCommand() )
      {
        case STARTED:
          std::cerr << "started" << std::endl;
          break;
        case TERMINATED:
          std::cerr << "terminated" << std::endl;
          break;
        case SUSPENDED:
          std::cerr << "suspended" << std::endl;
          break;
        case RESUMED:
          std::cerr << "resumed" << std::endl;
          break;
        case EVALUATED:
          std::cerr << "evaluated" << std::endl;
          break;
      }
#endif
      theEventSocket->send( lMessage.get(), length );
#ifndef NDEBUG
      std::clog << "[Server Thread] event sent" << std::endl;
#endif
    } catch( DebuggerSocketException &e ) {
      std::cerr << e.what() << std::endl;
    }
}


void ZorbaDebugger::startedEvent()
{
    StartedEvent lMessage;
    sendEvent( &lMessage );
}

void ZorbaDebugger::suspendedEvent( QueryLoc &aLocation, SuspensionCause aCause )
{
    SuspendedEvent lMessage( aLocation, aCause );
    sendEvent( &lMessage );
}

void ZorbaDebugger::resumedEvent()
{
    ResumedEvent lMessage;
    sendEvent( &lMessage );
}

void ZorbaDebugger::terminatedEvent()
{
    TerminatedEvent lMessage;
    sendEvent( &lMessage );
}

void ZorbaDebugger::runQuery()
{
  setStatus( QUERY_RUNNING );
  try
  {
    Zorba_SerializerOptions lSerOptions;
    lSerOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
    theQuery->serialize( *theOutputStream, *theSerOptions );
    std::cout.flush();
  } catch ( StaticException& se ) {
    std::cerr << se << std::endl;
  } catch ( DynamicException& de ) {
    std::cerr << de << std::endl;
  } catch ( SystemException& se ) {
    std::cerr << se << std::endl;
  }
  setStatus( QUERY_TERMINATED );
}

bool ZorbaDebugger::hasToSuspend()
{
  //If the query has been suspend by the user
  if( theStatus == QUERY_SUSPENDED ){ return true; }
  
  std::map<unsigned int, QueryLoc>::iterator it;
  //TODO: can be faster
  for ( it = theBreakpoints.begin(); it != theBreakpoints.end(); it++ )
  {
    if (it->second == theLocation)
    {
      setStatus( QUERY_SUSPENDED, CAUSE_BREAKPOINT );
      return true;
    }
  }
 
  std::map<unsigned int, xqpString>::iterator lIter;
  for ( lIter = theWatchpoints.begin(); lIter != theWatchpoints.end(); lIter++ )
  {
    std::auto_ptr< CompilerCB > ccb(new CompilerCB(*thePlanState->theCompilerCB));
    std::auto_ptr< dynamic_context > dctx(new dynamic_context(thePlanState->dctx()));
      
    //TODO: catch exceptions
    std::auto_ptr<PlanWrapperHolder> eval_plan = compileEvalPlan(theLocation, ccb.get(), dctx.get(), lIter->second, *thePlanState);
    PlanWrapper* lIterator = eval_plan->get();
    try
    {
      store::Item_t lItem;
      std::map<xqpString, xqpString> lValuesAndTypes;

      error::ErrorManager lErrorManger;

      lIterator->next(lItem);
      
      if(lItem->getEBV()->getBooleanValue())
      {
        setStatus(QUERY_SUSPENDED, CAUSE_BREAKPOINT);
	      return true;
      }
    } catch ( error::ZorbaError& e) {
      //do nothing... 
    }
  }
  return false;
}

/**
 * Perform an handshake with the client.
 * Simply returns the received first 11 bytes.
 */
void ZorbaDebugger::handshake( TCPSocket * aSock )
{
  ZorbaArrayAutoPointer<Byte> msg(new Byte[11]);
  try
  {
    aSock->recv( msg.get(), 11 );
    aSock->send( msg.get(), 11 );
  } catch( DebuggerSocketException &e ) {
    std::cerr << e.what() << std::endl;
  }
}

/**
 * TODO: check if the client has closed the connection
 */
void ZorbaDebugger::handleTcpClient( TCPSocket * aSock )
{
  ZorbaArrayAutoPointer<Byte> lByteMessage;
  std::auto_ptr<AbstractMessage> lMessage;
  std::auto_ptr<ReplyMessage> lReplyMessage;
  Length length;
  try
  {
    lMessage.reset(MessageFactory::buildMessage( aSock ));
    AbstractCommandMessage * lCommandMessage = dynamic_cast< AbstractCommandMessage * > ( lMessage.get() );
    if ( lCommandMessage )
    {
      //process the command message
      //TODO: flush the output somewhere else
      std::cout.flush();
      processMessage( lCommandMessage );
      //Send the reply message
      lReplyMessage.reset(lCommandMessage->getReplyMessage());
      lByteMessage.reset(lReplyMessage->serialize( length ));
      aSock->send( lByteMessage.get(), length );
    } else {
#ifndef NDEBUG
      std::clog << "[Server Thread] Received something wrong" << std::endl;
#endif
      //If something goes wrong, buildMessage() receive a Reply Message containing the error description
      //Send it back to the client right away
      lReplyMessage.reset(dynamic_cast<ReplyMessage *> ( lMessage.get() ));
      if( lReplyMessage.get() ){
        lByteMessage.reset(lReplyMessage->serialize( length ));
        aSock->send( lByteMessage.get(), length );
      } else {
        std::cerr << "[Server Thread] Internal error occured. Couldn't send the error message." << std::endl;
      }
    }    
  } catch ( std::exception &e ) {
#ifndef NDEBUG
    std::clog << "The connection with the client is closed" << std::endl;
    std::clog <<  e.what() << std::endl;
#endif
  }
}

void ZorbaDebugger::run()
{
  theRuntimeThread = new Thread( runtimeThread, this );
}

void ZorbaDebugger::suspend()
{
  setStatus( QUERY_SUSPENDED );
}

void ZorbaDebugger::resume()
{
  if ( theStatus == QUERY_SUSPENDED )
  {
    setStatus( QUERY_RUNNING );
    theRuntimeThread->resume();
  }
}

void ZorbaDebugger::terminate()
{
  setStatus( QUERY_TERMINATED );
}

std::stack<unsigned int> ZorbaDebugger::getCurrentDecimal() const
{
  std::stack<unsigned int> lCurrentDecimal;
  std::map< std::stack<unsigned int>, const QueryLoc>::const_iterator it;
  for(it=theClassification.begin(); it!=theClassification.end(); ++it)
  {
    if(it->second == theLocation)
    {
      lCurrentDecimal = it->first;
      break;
    }
  }
  return lCurrentDecimal;
}

void ZorbaDebugger::step( const StepCommand aKind )
{
  //find the current decimal.
  std::stack<unsigned int> lCurrentDecimal = getCurrentDecimal();
  //if the current decimal is not found something really wrong happened
  if(lCurrentDecimal.empty())
  {
    //TODO: proper error handling
    std::cerr << "Internal error occured for step." << std::endl;
    return;
  }
  if( aKind == STEP_OVER )
  {
    std::cerr << "Step Over." << std::endl;
    isSteppingOver = true;
    theDecimalSize = lCurrentDecimal.size();
    resume();
  } else if ( aKind == STEP_INTO ) {
    std::cerr << "Step Into." << std::endl;
    isSteppingInto = true;
    theDecimalSize = lCurrentDecimal.size()+1;
    resume();
  } else if ( aKind == STEP_OUT ) {
    std::cerr << "Step Out." << std::endl;
    isSteppingOut = true;
    lCurrentDecimal.pop();
    theDecimalSize = lCurrentDecimal.size();
    resume();
  }
}

bool ZorbaDebugger::hasToStepOver() const
{
  std::stack<unsigned int> lDecimal(getCurrentDecimal());
  if(lDecimal.empty())
  {
    return false;
  }
  if(isSteppingOver && lDecimal.size() <= theDecimalSize)
  {
    return true;
  }
  return false;
}

bool ZorbaDebugger::hasToStepInto() const
{
  std::stack<unsigned int> lDecimal(getCurrentDecimal());
  if(lDecimal.empty())
  {
    return false;
  }
  if(isSteppingInto) //&& lDecimal.size() == theDecimalSize)
  {
    return true;
  }
  return false;
}

bool ZorbaDebugger::hasToStepOut() const
{
  std::stack<unsigned int> lDecimal(getCurrentDecimal());
  if(lDecimal.empty())
  {
    return false;
  }
  if(isSteppingOut && lDecimal.size() <= theDecimalSize)
  {
    return true;
  }
  return false;
}

void ZorbaDebugger::stepOver()
{
  isSteppingOver = false;
}

void ZorbaDebugger::stepInto()
{
  isSteppingInto = false;
}

void ZorbaDebugger::stepOut()
{
  isSteppingOut = false;
}

void ZorbaDebugger::eval( xqpString anExpr )
{
  if ( theStatus != QUERY_IDLE && theStatus != QUERY_TERMINATED )
  {
    std::auto_ptr<EvaluatedEvent> lMsg;
    try {
      std::auto_ptr< CompilerCB > ccb(new CompilerCB(*thePlanState->theCompilerCB));
      std::auto_ptr< dynamic_context > dctx(new dynamic_context(thePlanState->dctx()));
      std::auto_ptr<PlanWrapperHolder> eval_plan = compileEvalPlan(theLocation, ccb.get(), dctx.get(), anExpr, *thePlanState);
      PlanWrapper* lIterator = eval_plan->get();
      assert(lIterator != 0);

      store::Item_t lItem;
      std::map<xqpString, xqpString> lValuesAndTypes;

      error::ErrorManager lErrorManger;
      serializer lSerializer(&lErrorManger);
      lSerializer.set_parameter("omit-xml-declaration", "yes");

      while (lIterator->next(lItem)) {
        std::ostringstream os;
        lSerializer.serialize(lItem, os);
        xqpString lValue = os.str();
        xqpString lType( lItem->getType()->getStringValue() );
        lValuesAndTypes.insert(std::pair<xqpString, xqpString>(lValue, lType));
      }
      lMsg.reset(new EvaluatedEvent(anExpr, lValuesAndTypes));
    } catch ( error::ZorbaError& e) {
      std::stringstream lOutputStream;
      xqpString lDescription = e.theDescription.replace("\\\"", "", "");
      lOutputStream << "Error: " << error::ZorbaError::toString(e.theErrorCode) << " " << lDescription;
      lMsg.reset(new EvaluatedEvent( anExpr, lOutputStream.str() ));
    }
    assert(lMsg.get() != 0);
    sendEvent(lMsg.get());
  }
}

const QueryLoc ZorbaDebugger::addBreakpoint(const QueryLoc& aLocation)
{
  xqpString lFilename = aLocation.getFilename();
  unsigned int lLineNumber = aLocation.getLineBegin();
  std::pair<std::stack<unsigned int>, QueryLoc> lBreakpoint;
  do{
    std::map<std::stack<unsigned int>, const QueryLoc>::iterator it;
    for(it=theClassification.begin(); it!=theClassification.end(); ++it)
    {
      if( it->second.getFilename() == lFilename &&
          it->second.getLineBegin() == lLineNumber &&
          lBreakpoint.first.empty())//(lBreakpoint.first.empty() || lBreakpoint.first.size() > it->first.size()))
      {
        lBreakpoint.first = it->first;
        lBreakpoint.second = it->second;  
      }
    }
    ++lLineNumber;
    //If a breakpoint hasn't been found 10 lines below, we return an empty location
    if( lLineNumber > 10 )
    {
      return lBreakpoint.second;
    }
  }while(lBreakpoint.first.empty());
  return lBreakpoint.second;
}

std::auto_ptr<PlanWrapperHolder>
ZorbaDebugger::compileEvalPlan(const QueryLoc& loc, CompilerCB* ccb, dynamic_context* dctx, const xqpString& anExpr, PlanState& planState)
{
  std::auto_ptr<PlanWrapperHolder> eval_plan(new PlanWrapperHolder());
  
  checked_vector< std::string > var_keys;

  //set up eval state's ccb
  ccb->m_sctx_list.push_back( ccb->m_sctx = ccb->m_sctx->create_child_context() );
  ccb->m_debugger = 0;

  eval_plan->reset(
    new PlanWrapper (
      EvalIterator::compile ( ccb, anExpr, theVarnames, theVartypes ),
      ccb,
      dctx,
      planState.theStackDepth + 1 )
    );
  eval_plan->get()->checkDepth( loc );
  
  for (unsigned i = 0; i < theChildren.size () - 1; i++)
  {
    //reset the plan iterator for multiple execution
    theChildren[i+1]->reset(*thePlanState);
    store::Iterator_t lIter = new PlanIteratorWrapper(theChildren[i+1], *thePlanState);
    // TODO: is saving an open iterator efficient?
    // Then again if we close theChildren [1] here,
    // we won't be able to re-open it later via the PlanIteratorWrapper
    dctx->add_variable(dynamic_context::var_key(ccb->m_sctx->lookup_var(theVarnames[i])), lIter);
  }
  return std::auto_ptr<PlanWrapperHolder>(eval_plan.release());
}

void ZorbaDebugger::processMessage(AbstractCommandMessage * aMessage)
{
  switch( aMessage->getCommandSet() )
  {
    case EXECUTION:
    {
      switch( aMessage->getCommand() )
      {
        case RUN:
        {
#ifndef NDEBUG
          RunMessage * lMessage = dynamic_cast<RunMessage *> ( aMessage );
          assert( lMessage );
#endif
          run();
          break;
        }
        case SUSPEND:
        {
#ifndef NDEBUG
          SuspendMessage * lMessage = dynamic_cast<SuspendMessage *> ( aMessage );
          assert( lMessage );
#endif
          suspend();
          break;
        }
        case RESUME:
        {
#ifndef NDEBUG
          ResumeMessage * lMessage = dynamic_cast<ResumeMessage*> ( aMessage );
          assert( lMessage );
#endif
          resume();
          break;
        }
        case TERMINATE:
        {
#ifndef NDEBUG
          TerminateMessage * lMessage = dynamic_cast<TerminateMessage *> ( aMessage );
          assert( lMessage );
#endif
          terminate();
          break;
        }
        case STEP:
        {
#ifndef NDEBUG
          StepMessage* lMessage = dynamic_cast<StepMessage *> ( aMessage );
          assert( lMessage );
#else
          StepMessage* lMessage =  static_cast<StepMessage *> ( aMessage );
#endif
          step(lMessage->getStepKind());
          break;
        }
        default: throw InvalidCommandException("Internal Error. Command not implemented for execution command set .");
    }
      break;
    case BREAKPOINTS:
    {
      switch( aMessage->getCommand() )
      {
        case SET:
        {
          SetMessage * lMessage;
#ifndef NDEBUG
          lMessage = dynamic_cast< SetMessage * > ( aMessage );
          assert( lMessage );
#else
          lMessage =  static_cast< SetMessage * > ( aMessage );
#endif
          SetReply* lReply = new SetReply(lMessage->getId(), DEBUGGER_NO_ERROR);
          
          std::map<unsigned int, QueryLoc> locations = lMessage->getLocations();
          std::map<unsigned int, QueryLoc>::iterator it;
          for ( it = locations.begin(); it != locations.end(); it++ )
          {
            QueryLoc loc = addBreakpoint(it->second);
            theBreakpoints.insert( std::make_pair( it->first, loc ) ); 
            lReply->addBreakpoint(it->first, loc);
          }
          
          std::map<unsigned int, xqpString> exprs = lMessage->getExprs();
          std::map<unsigned int, xqpString>::iterator lIt;
          for ( lIt = exprs.begin(); lIt != exprs.end(); lIt++ )
          {
            theWatchpoints.insert( std::make_pair( lIt->first, lIt->second ) ); 
          }

          lMessage->setReplyMessage(lReply);
          break;
        }
        case CLEAR:
        {
          ClearMessage * lMessage;
#ifndef NDEBUG
          lMessage = dynamic_cast< ClearMessage * > ( aMessage );
          assert( lMessage );
#else
          lMessage =  static_cast< ClearMessage * > ( aMessage );
#endif
          std::vector<unsigned int>::iterator it;
          std::vector<unsigned int> ids = lMessage->getIds();
          for ( it = ids.begin(); it != ids.end(); it++ )
          {
            std::map<unsigned int, QueryLoc>::iterator
            lIter = theBreakpoints.find( *it );
            if ( lIter !=  theBreakpoints.end() )
            {
              theBreakpoints.erase( lIter );
            } else {
              std::map<unsigned int, xqpString>::iterator lIter2 =
              theWatchpoints.find( *it );
              if ( lIter2 != theWatchpoints.end() ) {
                theWatchpoints.erase( lIter2 );
              }
            }
          }
          break;
        }
        default: throw InvalidCommandException("Internal Error. Command not implemented for breakpoints command set.");
      }
    }
      break;
    case ENGINE_EVENT:
    case STATIC:
    case DYNAMIC:
    {
      switch ( aMessage->getCommand() )
      {
        case EVAL:
        {
          EvalMessage * lMessage;
#ifndef NDEBUG
          lMessage = dynamic_cast< EvalMessage * > ( aMessage );
          assert( lMessage );
#else
          lMessage = static_cast< EvalMessage * > ( aMessage );
#endif
          eval( lMessage->getExpr() );
          break;
        }
        case VARIABLES:
        {
          VariableMessage * lMessage;
#ifndef NDEBUG
          lMessage = dynamic_cast< VariableMessage * > ( aMessage );
#else
          lMessage = static_cast< VariableMessage * > ( aMessage );
#endif
          VariableReply * lReply = new VariableReply( lMessage->getId(), DEBUGGER_NO_ERROR );
          for( unsigned i = 0; i<theVarnames.size(); i++ )
          {
            xqpString lName(theVarnames.at(i)->getStringValue());
            xqpString lType;
            if( theVartypes.at(i) != 0 )
            {
              lType = theVartypes.at(i)->toString();
            } else {
              lType = "[XS_UNTYPED]";
            }
            checked_vector<global_binding>::iterator it;
            bool is_global = false;
            for ( it = theGlobals.begin(); it != theGlobals.end(); ++it )
            {
              if ( it->first->get_varname() == theVarnames.at(i) )
              {
                lReply->addGlobal( lName, lType );
                is_global = true;
                break;
              }
            }
            if ( ! is_global )
            {
              lReply->addLocal( lName, lType );
            }
          }
          lMessage->setReplyMessage( lReply );
          break;
        }
        case FRAME:
        {
          FrameMessage* lMessage;
#ifndef NDEBUG
          lMessage = dynamic_cast<FrameMessage*>(aMessage);
#else
          lMessage = static_cast<FrameMessage*>(aMessage);
#endif
          FrameReply* lReply = new FrameReply(lMessage->getId(), DEBUGGER_NO_ERROR, theStack);
          lMessage->setReplyMessage(lReply);
          break;
        }
        default: throw InvalidCommandException("Internal Error. Command not implemented for dynamic command set.");
      }
    }
    break;
    default:
      throw InvalidCommandException("Internal Error. CommandSet not implemented.");
  }
}
};
}//end of namespace

