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
#ifndef ZORBA_DEBUGGER_COMMONS_H
#define ZORBA_DEBUGGER_COMMONS_H

#include <map>
#include <string>

#include "compiler/parser/query_loc.h"
#include "runtime/core/item_iterator.h"

#include "debugger/debugger_common.h"
#include "zorbaserialization/serialization_engine.h"
#include "zorbatypes/zstring.h"

struct Zorba_SerializerOptions;
typedef struct Zorba_SerializerOptions Zorba_SerializerOptions_t;

namespace zorba {
  class dynamic_context;
  class static_context;
  class DebuggerRuntime;
  class ZorbaDebugIterator;
  class TranslatorImpl;
  class PlanState;
  class ZorbaDebugIteratorState;
  struct DebugLocation;
  typedef struct DebugLocation DebugLocation_t;
  struct DebugLocation : public zorba::serialization::SerializeBaseClass{
    std::string theFileName;
    unsigned long theLineNumber;
    QueryLoc theQueryLocation;
    public:
    DebugLocation() {}
    SERIALIZABLE_CLASS(DebugLocation)
    SERIALIZABLE_CLASS_CONSTRUCTOR(DebugLocation)
    void serialize(::zorba::serialization::Archiver& ar);

    bool operator()(const DebugLocation_t& aLocation1,
      const DebugLocation_t& aLocation2) const;
  };

  /**
  * @brief This class is used for the eval command.
  *
  * This class is just a SingeltonIterator, which gives the client
  * the possibility to reset the stored item.
  */
  class DebuggerSingletonIterator : 
    public NoaryBaseIterator<DebuggerSingletonIterator,PlanIteratorState>
  {
  public: //Constructor and Destructor
    /**
    * @brief The default constructor only takes the static context.
    */
    DebuggerSingletonIterator(static_context* sctx,
      QueryLoc loc,
      DebuggerCommons* lCommons);

    virtual ~DebuggerSingletonIterator() {}

  public: //Implementation

    void accept(PlanIterVisitor& v) const;

    bool nextImpl(store::Item_t& result, PlanState& planState) const;

  protected:
    DebuggerCommons* theCommons;
  public:
    SERIALIZABLE_CLASS(DebuggerSingletonIterator)
    SERIALIZABLE_CLASS_CONSTRUCTOR2T(DebuggerSingletonIterator, NoaryBaseIterator<DebuggerSingletonIterator,PlanIteratorState>)
    void serialize(::zorba::serialization::Archiver& ar);
  };

  /**
  * @brief A class used as common shared object between the debugger runtime
  *  and the debug iterators.
  *
  * This class is by the iterators to share information, get commands from
  * the debug runtime and deliver information to the debugger runtime.
  *
  */
  class DebuggerCommons : public zorba::serialization::SerializeBaseClass{
    friend class TranslatorImpl;
  public: // Creation and destruction
    /**
    * @brief The default constructor.
    *
    * Initializes the internally used variables.
    */
    DebuggerCommons(static_context* sctx);
    virtual ~DebuggerCommons();

    SERIALIZABLE_CLASS(DebuggerCommons);
    SERIALIZABLE_CLASS_CONSTRUCTOR(DebuggerCommons);
    void serialize( ::zorba::serialization::Archiver& ar);

  public: // Commands
    //************************************
    // Method:    setRuntime
    // FullName:  zorba::DebuggerCommons::setRuntime
    // Access:    public 
    // Returns:   void
    // Qualifier:
    // Parameter: DebuggerRuntime * aRuntime
    //************************************
    void setRuntime(DebuggerRuntime* aRuntime);
    //************************************
    // Method:    setCurrentDynamicContext
    // FullName:  zorba::DebuggerCommons::setCurrentDynamicContext
    // Access:    public 
    // Returns:   void
    // Qualifier:
    // Parameter: dynamic_context * aDynamicContext
    //************************************
    void setCurrentDynamicContext(dynamic_context* aDynamicContext);
    void setCurrentStaticContext(static_context* aStaticContext);
    /**
    * @brief Adds a mapping from a uri to a filename.
    */
    void addModuleUriMapping(std::string aUri, std::string aFileUri);
    /**
    * @brief Sets the theBreak variable.
    *
    * @see DebuggerCommons::hasToBreak for a description why this is
    *  needed.
    *
    * @pre lBreak implies aCause != 0
    * @post theBreak == lBreak
    * @post theCause == aCause
    */
    void setBreak(bool lBreak, SuspensionCause aCause = 0);

    /**
    * @brief Sets the current iterator which can then be used by the
    *  debugger runtime.
    *
    * @param aIterator A pointer to the current iterator.
    * @post aIterator == theCurrentIterator
    */
    void setCurrentIterator(const ZorbaDebugIterator* aIterator);

    /**
    * @brief Sets the current planstate.
    *
    * When the debugger suspends, it saves the planstate in the commons.
    *
    * @param aPlanState a pointer to the current plan state.
    * @post thePlanState == aPlanState
    */
    void setPlanState(PlanState* aPlanState);

    /**
    * @brief Sets the current debugger state.
    *
    * When the debugger suspends, it saves its state.
    *
    * @param aState a pointer to the current debugger state.
    * @post aState == theDebugIteratorState
    */
    void setDebugIteratorState(ZorbaDebugIteratorState* aState);

    /**
    * @brief Sets a setpoint according to the step out rules.
    *
    * This method sets a breakpoint according to the rules according to
    * the step out rules.
    *
    * @pre theCurrentIterator != NULL
    */
    void makeStepOut();

    /**
    * @brief Sets a setpoint according to the step over rules.
    *
    * @pre theCurrentIterator != NULL
    */
    void makeStepOver();

    /**
    * @brief Evaluates aExpr and returns the result.
    *
    * @returns the result of the expression in the current context.
    */
    std::list<std::pair<zstring, zstring> > 
      eval(const zstring& aExpr, Zorba_SerializerOptions& aSerOpts);
  public: //Queries
    /**
    * Adds a breakpoint and then sets theLocation from aLocation
    * to the QueryLoc object, where it has to break.
    *
    * @param aLocation The location, where the debugger has to break.
    * @param aId The identifier of this breakpoint.
    *
    * @return true, if a breakable expression was found - false otherwise
    */
    bool addBreakpoint(DebugLocation_t& aLocation, unsigned int aId);
    /**
    * @brief Clears the breakpoint with the id aId.
    */
    void clearBreakpoint(unsigned int aId);
    //************************************
    // Method:    getRuntime
    // FullName:  zorba::DebuggerCommons::getRuntime
    // Access:    public 
    // Returns:   DebuggerRuntime*
    // Qualifier:
    //************************************
    DebuggerRuntime* getRuntime();
    //************************************
    // Method:    getCurrentDynamicContext
    // FullName:  zorba::DebuggerCommons::getCurrentDynamicContext
    // Access:    public 
    // Returns:   dynamic_context*
    // Qualifier: const
    //************************************
    dynamic_context* getCurrentDynamicContext() const;
    //************************************
    // Method:    getCurrentStaticContext
    // FullName:  zorba::DebuggerCommons::getCurrentStaticContext
    // Access:    public 
    // Returns:   static_context*
    // Qualifier: const
    //************************************
    static_context* getCurrentStaticContext() const;
    /**
    * @brief Returns true, if a breakpoint is set to the given location -
    * false otherwise.
    */
    bool hasToBreakAt(const QueryLoc& aLocation) const;
    /**
    * @brief Returns true, if a breakpoint is set to the given debugger
    * iterator - false otherwise.
    *
    * This method is used for step out and step over commands. It also
    * deletes the debugger from the list of step points, as soon, as
    * this function is called.
    *
    * @brief aIter != NULL
    */
    bool hasToBreakAt(const ZorbaDebugIterator* aIter);
    /**
    * @brief This returns true, if the runtime wants to suspend.
    *
    * If the debugger runtime wants to suspend, it sets theBreak to true,
    * which should be checked by every debug iterator on every iteration
    * step. If the variable is set to true, the iterator should suspend
    * the runtime immediately.
    *
    * There can be several reasons for the runtime for setting theBreak
    * to true. It could be a step in command from the user (where this
    * mechanism is used for convenience) or an interrupt from the user.
    *
    * @retval aCause sets aCause to the cause, why the debugger runtime
    *  has to suspend.
    * @return true, if the runtime wants the debugger to suspend.
    * @pre *aCause == 0
    * @post *aCause == theCause
    * @post Result == theBreak
    */
    bool hasToBreak(SuspensionCause* aCause) const;

    /**
    * @brief Gets the current debugger iterator.
    */
    const ZorbaDebugIterator* getCurrentIterator() const;

    /**
    * @brief Gets the item, which is used for all eval iterators of the debug
    * iterators.
    */
    store::Item_t getEvalItem();

    /**
    * @brief Gets a file path of a uri.
    *
    * This function first looks for a file uri associated with this uri
    * and then, returns a file path represented by this uri. If no file uri
    * is associated with aUri, this method assumes aUri is a file uri and
    * transforms it into a file path.
    */
    std::string getFilepathOfURI(const std::string& aUri) const;

  private:
    std::map<DebugLocation_t, bool, DebugLocation> theLocationMap;
    std::map<std::string, std::string >            theUriFileMappingMap;
    std::map<unsigned int, DebugLocation_t>        theBreakpoints;
    DebuggerRuntime*                               theRuntime;
    static_context*                                theCurrentStaticContext;
    dynamic_context*                               theCurrentDynamicContext;
    bool                                           theBreak;
    SuspensionCause                                theCause;
    /*const */ZorbaDebugIterator*                  theCurrentIterator;
    /**
    * @brief The list of step expressions.
    */
    std::list</*const*/ ZorbaDebugIterator*>       theBreakIterators;
    PlanState*                                     thePlanState;
    ZorbaDebugIteratorState*                       theDebugIteratorState;
    store::Item_t                                  theEvalItem;
    bool                                           theExecEval;
  };
}

#endif // ZORBA_DEBUGGER_COMMONS_H