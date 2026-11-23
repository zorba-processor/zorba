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


# Compute results directory for testdriver
SET (_results_dir "${CMAKE_CURRENT_BINARY_DIR}/QueryResults")

# collect all queries (suffix .xq) in all subdirectories of the Queries dir
FILE(GLOB_RECURSE TESTFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/Queries/ *.xq)

MESSAGE(STATUS "Adding SAX2 tests for CTest")
SET(TESTCOUNTER 0)

FOREACH(TESTFILE ${TESTFILES})

 STRING(LENGTH ${TESTFILE} TESTFILELENGTH)
 MATH(EXPR TESTLENGTH '${TESTFILELENGTH}-3' )
 STRING(SUBSTRING ${TESTFILE} 0 ${TESTLENGTH} TESTNAME)

 SET (TESTNAME "test/sax2/${TESTNAME}")
 ZORBA_ADD_TEST(${TESTNAME} sax2testdriver ${TESTFILE})

 # Create directory for output, if it doesn't exist
 GET_FILENAME_COMPONENT(_bucket_path ${TESTFILE} PATH)
 IF (NOT EXISTS "${_results_dir}/${_bucket_path}")
   FILE (MAKE_DIRECTORY "${_results_dir}/${_bucket_path}")
 ENDIF (NOT EXISTS "${_results_dir}/${_bucket_path}")

 ZORBA_SET_TEST_PROPERTY (${TESTNAME} TIMEOUT "10")

 MATH(EXPR TESTCOUNTER ${TESTCOUNTER}+1)
 MATH(EXPR TESTMOD "${TESTCOUNTER}/1000")
 IF (${TESTMOD})
   MESSAGE(STATUS "Adding another 1000 Tests")
   SET(TESTCOUNTER 0)
 ENDIF (${TESTMOD})
                
ENDFOREACH(TESTFILE)

# ZORBA_OPTLEVEL=O2 compiles at O2. The run writes the same result file as the
# run at the default level, so the two do not run at the same time.
ZORBA_ADD_TEST(test/sax2/sax2_test1-O2 sax2testdriver sax2_test1.xq)
SET_TESTS_PROPERTIES(test/sax2/sax2_test1-O2 PROPERTIES
  ENVIRONMENT "ZORBA_OPTLEVEL=O2"
  FAIL_REGULAR_EXPRESSION "optimization level O[01]"
  RESOURCE_LOCK sax2_test1 TIMEOUT 10)
SET_TESTS_PROPERTIES(test/sax2/sax2_test1 PROPERTIES RESOURCE_LOCK sax2_test1)
