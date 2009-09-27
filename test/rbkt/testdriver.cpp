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
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>


#include "testdriverconfig.h" // SRC and BIN dir definitions
#include "specification.h" // parsing spec files
#include "testuriresolver.h"
#include "testdriver_common.h"

#include "util/properties.h"

#include <zorba/static_context.h>
#include <zorba/util/file.h>

#include <zorbatypes/URI.h>
#include <zorbautils/strutil.h>

#include "zorba_test_setting.h"

#ifndef ZORBA_MINIMAL_STORE
#include <simplestore/simplestore.h>
#include "store/naive/simple_store.h"
#else
#include "store/minimal/min_store.h"
#endif

#include "testdriver_comparator.h"


//#define ZORBA_TEST_PLAN_SERIALIZATION

#define EXPECTED_ERROR  0
#define UNEXPECTED_ERROR  6

std::string rbkt_src_dir = zorba::RBKT_SRC_DIR;
std::string rbkt_bin_dir = zorba::RBKT_BINARY_DIR;
std::string w3c_ts = "w3c_testsuite/";


void loadProperties () 
{
  zorba::Properties::load(0, NULL);
}



int analyzeError (const Specification &lSpec, const TestErrorHandler& errHandler) 
{
  if (isErrorExpected(errHandler, &lSpec)) {
    printErrors(errHandler, "The following execution error occurred as expected", false);
    return EXPECTED_ERROR;
  } else {
    printErrors(errHandler, "Unexpected error executing query", false);
    std::cout << "Expected error(s):";
    for (std::vector<std::string>::const_iterator lIter = lSpec.errorsBegin();
         lIter != lSpec.errorsEnd(); ++lIter)
      {
        std::cout << " " << *lIter;
      }
    if (lSpec.errorsSize () == 0) { std::cout << " (none)"; }
    std::cout << std::endl;
    return UNEXPECTED_ERROR;
  }
}


int
#ifdef _WIN32_WCE
_tmain(int argc, _TCHAR* argv[])
#else
main(int argc, char** argv)
#endif
{
  if ( argc < 2 )
  {
    std::cout << "\nusage:   testdriver [testfile_1] [testfile_2] ..." << std::endl;
    return 1;
  }

  int errors;
  Specification lSpec;

  loadProperties ();

  // Instantiate the store and the zorba query processor
#ifdef ZORBA_MINIMAL_STORE
  zorba::storeminimal::SimpleStore* store =
  zorba::storeminimal::SimpleStore::getInstance();
#else
  zorba::simplestore::SimpleStore* store =
  zorba::simplestore::SimpleStoreManager::getStore();
#endif
  if (store == NULL) return 20;

  zorba::Zorba * engine = zorba::Zorba::getInstance(store);
  if (engine == NULL) return 21;

  int i = 1;
  for (;;) 
  {
    if (strcmp (argv [i], "--rbkt-src") == 0) {
      rbkt_src_dir = argv [i + 1];
      i += 2;
    } else if (strcmp (argv [i], "--rbkt-bin") == 0) {
      rbkt_bin_dir = argv [i + 1];
      i += 2;
    } else break;
  }

  zorba::XQuery_t lQuery;
  TestErrorHandler errHandler;

  DriverContext driverContext;
  driverContext.theEngine = engine;
  driverContext.theRbktSourceDir = rbkt_src_dir;
  driverContext.theSpec = &lSpec;

  for (int testcnt = 1; i < argc; ++i, ++testcnt)
  {
    std::string Queriesdir = "/Queries/";

    int path_flags = zorba::file::CONVERT_SLASHES | zorba::file::RESOLVE;

    std::string lQueryFileString = rbkt_src_dir + Queriesdir + argv[i];

#ifndef ZORBA_TEST_PLAN_SERIALIZATION_EXECUTION_ONLY
    // Form the full pathname for the file containing the query and make sure
    // that the file exists.
    zorba::file lQueryFile (lQueryFileString, path_flags);

    if ( (! lQueryFile.exists ()) || lQueryFile.is_directory () ) 
    {
      std::cout << "\n query file " << lQueryFile.get_path() 
                << " does not exist or is not a file" << std::endl;
      return 2;
    }
#endif//ZORBA_TEST_PLAN_SERIALIZATION_EXECUTION_ONLY

    // Check if this is w3c_testsuite test.
    std::string path = lQueryFileString;
    bool isW3Ctest = ( path.find ( "w3c_testsuite" ) != std::string::npos );
    std::string lQueryWithoutSuffix = 
    std::string(argv[i]).substr( 0, std::string(argv[i]).rfind('.') );
    std::auto_ptr<zorba::TestSchemaURIResolver>      resolver;
    std::auto_ptr<zorba::TestModuleURIResolver>      mresolver;
    std::auto_ptr<zorba::TestCollectionURIResolver>  cresolver;
    // Create the static context. If this is a w3c query, install special uri
    // resolvers in the static context.
    zorba::StaticContext_t lContext = engine->createStaticContext();

    if ( isW3Ctest ) 
    {
  #ifndef MY_D_WIN32
      std::string w3cDataDir = "/Queries/w3c_testsuite/TestSources/";
  #else
      std::string w3cDataDir = "/Queries/TestSources/";
  #endif
      std::string uri_map_file = rbkt_src_dir + w3cDataDir + "uri.txt";
      std::string mod_map_file = rbkt_src_dir + w3cDataDir + "module.txt";
      std::string col_map_file = rbkt_src_dir + w3cDataDir + "collection.txt";

      resolver.reset(new zorba::TestSchemaURIResolver(uri_map_file.c_str()));

      mresolver.reset(new zorba::TestModuleURIResolver(mod_map_file.c_str(),
                                                       lQueryWithoutSuffix));

      cresolver.reset(new zorba::TestCollectionURIResolver(col_map_file.c_str(),
                                                           rbkt_src_dir));
      lContext->setSchemaURIResolver ( resolver.get() );
      lContext->setModuleURIResolver ( mresolver.get() );
      lContext->setCollectionURIResolver ( cresolver.get() );
    }

    // Form the full pathname for the files that will receive the result or the
    // errors of this query. Then, delete these files if they exist already from
    // previous runs of the query. Finaly, create (if necessary) all the dirs
    // in the pathname of the result and error files.

    std::cout << "test " << lQueryWithoutSuffix << std::endl;

#ifndef ZORBA_TEST_PLAN_SERIALIZATION_COMPILE_ONLY
    zorba::file lResultFile (rbkt_bin_dir + "/QueryResults/" 
                             + lQueryWithoutSuffix + ".xml.res", path_flags);

    zorba::file lErrorFile  (rbkt_bin_dir + "/" 
                             + lQueryWithoutSuffix + ".err", path_flags);

    if ( lResultFile.exists () ) { lResultFile.remove (); }
    if ( lErrorFile.exists () )  { lErrorFile.remove ();  }

    zorba::file lBucket (lResultFile.branch_path());
    if ( ! lBucket.exists () )
      lBucket.deep_mkdir (); // create deep directories

    // Form the full pathname for the .spec file that may be associated
    // with this query. If the .spec file exists, read its contents to
    // extract args to be passed to the query (e.g., external var bindings),
    // exprected errors, or the pathnames of reference-result files.
    zorba::file lSpecFile(rbkt_src_dir + "/Queries/" + lQueryWithoutSuffix + ".spec",
                          path_flags);

    if ( lSpecFile.exists ()) 
      lSpec.parseFile(lSpecFile.get_path());

    // Get the pathnames of the reference-result files found in the .spec
    // file (if any).
    std::vector<zorba::file> lRefFiles;
    bool lRefFileExists = false;
    for (std::vector<std::string>::const_iterator lIter = lSpec.resultsBegin();
         lIter != lSpec.resultsEnd();
         ++lIter) 
    {
      std::string lTmp = *lIter;
      zorba::str_replace_all(lTmp, "$RBKT_SRC_DIR", rbkt_src_dir);
      zorba::file lRefFile(lTmp, path_flags);
      if (lRefFile.exists()) 
      {
        lRefFileExists = true;
      }
      else
      {
        std::cout << "Warning: missing reference result file " 
                  << lRefFile.get_path () << std::endl;
      }
      lRefFiles.push_back(lRefFile);
    }

    // If no ref-results file was specified in the .spec file, create a default
    // finename for that file. For w3c tests, the ref file is the same for
    // xqueryx and xquery tests, hence, we remove the string xqueryx or xquery
    // from the path
    if (lRefFiles.size () == 0) 
    {
      std::string lRefFileTmpString = lQueryWithoutSuffix;

      if (isW3Ctest) 
      {
        if (lQueryWithoutSuffix.find("XQueryX") != std::string::npos)
          lRefFileTmpString = lRefFileTmpString.erase(14, 8);
        else if (lQueryWithoutSuffix.find("XQuery") != std::string::npos)
          lRefFileTmpString = lRefFileTmpString.erase(14, 7);
      }
  
      lRefFiles.push_back(zorba::file(rbkt_src_dir + "/ExpQueryResults/" +
                                      lRefFileTmpString + ".xml.res"));

      if (lRefFiles [0].exists())
        lRefFileExists = true;
    }
#endif//ZORBA_TEST_PLAN_SERIALIZATION_COMPILE_ONLY

#ifndef ZORBA_TEST_PLAN_SERIALIZATION_EXECUTION_ONLY
    // print the query
    std::cout << "Query:" << std::endl;
    zorba::printFile(std::cout, lQueryFile.get_path());
    std::cout << std::endl;


    // create and compile the query
    std::string lQueryString;
    slurp_file(lQueryFile.get_path().c_str(), lQueryString, rbkt_src_dir, rbkt_bin_dir);

    lQuery = engine->createQuery (&errHandler);
    lQuery->setFileName (lQueryFile.get_path ());
#if 1
    lQuery->compile (lQueryString.c_str(), lContext, getCompilerHints());
#else
    Zorba_CompilerHints lHints;
    lHints.opt_level = ZORBA_OPT_LEVEL_O0;
    lQuery->compile (lQueryString.c_str(), lContext, lHints);
#endif

    errors = -1;
    if ( errHandler.errors() )
    {
      errors = analyzeError (lSpec, errHandler);
      if( errors == UNEXPECTED_ERROR )
        return 6;
      std::cout << "testdriver: success" << std::endl;
      return 0;
    } 
    // no compilation errors

#ifdef ZORBA_TEST_PLAN_SERIALIZATION
    try
    {
      clock_t t0, t1;
      std::string binary_path = lQueryFileString;
      binary_path = binary_path.substr( 0, binary_path.rfind('.') );
      binary_path += ".plan";
      t0 = clock();
      std::ofstream fbinary(binary_path.c_str(), std::ios_base::binary);
      if(!lQuery->saveExecutionPlan(fbinary, ZORBA_USE_BINARY_ARCHIVE))
      {
        printf("save execution plan FAILED\n");
        return 0x0badc0de;
      }
      fbinary.close();
      t1 = clock();
      printf("save execution plan in %f sec\n", (float)(t1-t0)/CLOCKS_PER_SEC);
    }
    catch(zorba::ZorbaException &err)
    {
      std::cout << err << std::endl;
      return -1;
    }
#endif//ZORBA_TEST_PLAN_SERIALIZATION

#ifdef ZORBA_TEST_PLAN_SERIALIZATION_COMPILE_ONLY
    continue;
#endif//ZORBA_TEST_PLAN_SERIALIZATION_COMPILE_ONLY

#endif//#ifndef ZORBA_TEST_PLAN_SERIALIZATION_EXECUTION_ONLY

#ifdef ZORBA_TEST_PLAN_SERIALIZATION
    try
    {
      clock_t t0, t1;
      std::string binary_path = lQueryFileString;
      binary_path = binary_path.substr( 0, binary_path.rfind('.') );
      binary_path += ".plan";
      lQuery = engine->createQuery (&errHandler);
      t0 = clock();
      std::ifstream   ifbinary(binary_path.c_str(), std::ios_base::binary);
      if(!ifbinary.is_open())
      {
        std::cout << "cannot open plan " << binary_path << std::endl;
        return 0;
      }
      if ( isW3Ctest ) 
      {
        lQuery->loadExecutionPlan(ifbinary, NULL, cresolver.get(), resolver.get(), mresolver.get());
      }
      else
      {
        lQuery->loadExecutionPlan(ifbinary);
      }
      t1 = clock();
      printf("load execution plan in %f sec\n", (float)(t1-t0)/CLOCKS_PER_SEC);
    }
    catch(zorba::ZorbaException &err)
    {
      std::cout << err << std::endl;
      return -1;
    }
#endif

    // Create dynamic context and set in it the external variables, the current
    // date & time, and the timezone.
    createDynamicContext(driverContext, lContext, lQuery);

    errors = -1;
    {
      { // serialize xml
        std::ofstream lResFileStream(lResultFile.get_path().c_str());
        assert (lResFileStream.good());
        Zorba_SerializerOptions lSerOptions;
        lSerOptions.ser_method = ZORBA_SERIALIZATION_METHOD_XML;
        lSerOptions.omit_xml_declaration = ZORBA_OMIT_XML_DECLARATION_YES;
        lSerOptions.indent = ZORBA_INDENT_NO;
        
        lQuery->serialize(lResFileStream, &lSerOptions);
      }
      
      if (lSpec.errorsSize() == 0 && ! lRefFileExists )
      {
        std::cout << "No reference result and no expected errors." << std::endl;
        return 3;
      }
      
      if (errHandler.errors())
      {
        errors = analyzeError (lSpec, errHandler);
      }
      else if ( lSpec.errorsSize() > 0 ) 
      {
        if ( ! lRefFileExists ) 
        {
          std::cout << "Expected error(s)";
          for (std::vector<std::string>::const_iterator lIter = lSpec.errorsBegin();
               lIter != lSpec.errorsEnd(); ++lIter)
          {
            std::cout << " " << *lIter;
          }
          if ( lResultFile.exists () && lResultFile.get_size () == 0)
          {
            std::cout << " but got empty result" << std::endl;
          }
          else 
          {
            std::cout << " but got result:" << std::endl;
            zorba::printFile(std::cout, lResultFile.get_path());
            std::cout << "=== end of result ===" << std::endl;
          }
          return 7;
        }
      }
    }

    if( errors == UNEXPECTED_ERROR)
    {
      return 6;
    }
    else if( errors == -1 ) 
    {
      std::cout << "Result:" << std::endl;
      zorba::printFile(std::cout, lResultFile.get_path());
      std::cout << "=== end of result ===" << std::endl;
      std::cout.flush();
      size_t i = 1;
      for (std::vector<zorba::file>::const_iterator lIter = lRefFiles.begin();
           lIter != lRefFiles.end(); ++lIter) 
      {
        int lLine, lCol, lPos; // where do the files differ
        std::string lRefLine, lResultLine;
        bool lRes = zorba::fileEquals(lIter->c_str(),
                                      lResultFile.c_str(),
                                      lLine, lCol, lPos,
                                      lRefLine, lResultLine);
        if (lRes) 
        {
          std::cout << "testdriver: success (non-canonical result # " << i 
                    << " matches)" << std::endl;
          return 0;
        }

        std::cout << "testdriver: non-canonical result for reference result # " 
                  << i << " doesn't match." << std::endl;

        int lCanonicalRes = zorba::canonicalizeAndCompare(lSpec.getComparisonMethod(),
                                                          lIter->c_str(),
                                                          lResultFile.c_str(),
                                                          rbkt_bin_dir);
        if (lCanonicalRes == 0) 
        {
          std::cout << "testdriver: success (canonical result # " << i  
                    << " matches)" << std::endl;
          return 0;
        }
        std::cout << "testdriver: canonical result for reference result # " << i 
                  << " doesn't match." << std::endl;
        ++i;
      } // for 
      
      std::cout << "testdriver: none of the reference results matched" << std::endl;
      return 8;
    }
  }
  std::cout << "testdriver: success" << std::endl;
  return 0;
}
