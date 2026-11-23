# Copyright 2006-2016 zorba.io
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Multithreaded API tests (label "mt"), built only when the engine is built
# for several threads (ZORBA_FOR_ONE_THREAD_ONLY=OFF). They check results in
# every build and report races under ThreadSanitizer (the tsan-mt job).

SET(MTSchemaTests)
IF (NOT ZORBA_NO_XMLSCHEMA)
  SET(MTSchemaTests mt_schema.cpp mt_lock_order.cpp)
ENDIF (NOT ZORBA_NO_XMLSCHEMA)

CREATE_TEST_SOURCELIST(MTTests
  MTTests.cpp
  mt_independent.cpp
  mt_globals.cpp
  mt_clone.cpp
  mt_shared_sctx.cpp
  mt_shared_items.cpp
  mt_store_rw.cpp
  mt_doc_cache.cpp
  mt_lifecycle.cpp
  mt_node_reference.cpp
  mt_node_dereference.cpp
  mt_lock_upgrade.cpp
  mt_prolog_program.cpp
  mt_function_coercion.cpp
  mt_function_cache.cpp
  mt_function_lookup.cpp
  mt_external_module.cpp
  mt_query_sctx.cpp
  mt_module_instance.cpp
  mt_function_items.cpp
  mt_shared_handles.cpp
  ${MTSchemaTests}
)

ZORBA_GENERATE_EXE(MTTests "${MTTests}" "" "" "")
# mt_external_module loads a module shipped with Zorba (the file module):
# zorbacmd depends on the core module libraries and their URI files.
ADD_DEPENDENCIES(MTTests zorbacmd)

SET (TestsToRun ${MTTests})
LIST (FILTER TestsToRun EXCLUDE REGEX "(^|/)MTTests\\.cpp$")

FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  SET (TestName "test/api/mt/${TName}")
  ZORBA_ADD_TEST(${TestName} MTTests ${TName})
  # A crash under ThreadSanitizer can leave the process spinning in its
  # signal handler; the timeout kills it.
  SET_TESTS_PROPERTIES(${TestName} PROPERTIES LABELS mt TIMEOUT 300)
ENDFOREACH(test)

# mt_clone without the run before cloning: the threads generate the plans of
# the user functions concurrently.
ZORBA_ADD_TEST(test/api/mt/mt_clone_cold MTTests mt_clone cold)
SET_TESTS_PROPERTIES(test/api/mt/mt_clone_cold PROPERTIES LABELS mt TIMEOUT 300)
