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

ADD_DEFINITIONS(-DZORBA_HTTP_TEST_SERVER_DEFAULT="${ZORBA_HTTP_TEST_SERVER}")

# schema_utf8 needs XML Schema support (Xerces-C)
SET(SchemaTests)
IF (NOT ZORBA_NO_XMLSCHEMA)
  SET(SchemaTests schema_utf8.cpp)
ENDIF (NOT ZORBA_NO_XMLSCHEMA)

CREATE_TEST_SOURCELIST(APITests
  APITests.cpp
  itemfactory.cpp
  xmldatamanager.cpp
  jsondatamanager.cpp
  xquery.cpp
  test_static_context.cpp
  test_dynamic_context.cpp
  serializer.cpp
  annotations.cpp
  external_function_errors.cpp
  userdefined_uri_resolution.cpp
  validate.cpp
  security_policy.cpp
  secure_default.cpp
  xml_parser_security.cpp
  uri_access_security.cpp
  evaluation_limits.cpp
  expr_depth_limit.cpp
  module_security.cpp
  invoke_module_globals.cpp
  ${SchemaTests}
)

# add the executable
ZORBA_GENERATE_EXE(APITests "${APITests}" "" "" "")

# remove the test driver source file
SET (TestsToRun ${APITests})
LIST (FILTER TestsToRun EXCLUDE REGEX "(^|/)APITests\\.cpp$")

MESSAGE(STATUS "Adding API tests for CTest")

# add all the ADD_TEST for each testing
FOREACH (test ${TestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  SET (TestName "test/api/${TName}")
  ZORBA_ADD_TEST(${TestName} APITests ${TName}
    "${CMAKE_BINARY_DIR}/TEST_URI_PATH")
ENDFOREACH(test)

# Schema import/validation with non-ASCII names must not depend on the process
# locale (the test also switches LC_CTYPE to a non-UTF-8 locale if available).
IF (NOT ZORBA_NO_XMLSCHEMA)
  SET_TESTS_PROPERTIES("test/api/schema_utf8" PROPERTIES
    ENVIRONMENT "LC_ALL=C;LANG=C")
ENDIF (NOT ZORBA_NO_XMLSCHEMA)

# Add importing schema for userdefined_uri_resolution test
DECLARE_ZORBA_SCHEMA(FILE import-hello.xsd
  URI http://www.zorba-xquery.com/import-hello TEST_ONLY)

EXPECTED_FAILURE(test/api/test_static_context 3118348)
