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

# scripts/ast2dot.xq: zorba --print-ast output -> Graphviz DOT (no Graphviz needed)
ADD_TEST(NAME "scripts/ast2dot"
  COMMAND "${CMAKE_COMMAND}"
    -D "ZORBA_EXE=${ZORBA_EXE}"
    -D "SRC_DIR=${CMAKE_SOURCE_DIR}"
    -D "WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/ast2dot.cmake")
SET_TESTS_PROPERTIES("scripts/ast2dot" PROPERTIES PASS_REGULAR_EXPRESSION "Passed!")

# bench/compare.sh and bench/compare_table.xq: the comparison of two builds on a
# benchmark suite (.github/workflows/bench.yml). The table script runs on a
# sample of raw results everywhere; the driver runs on Linux, where it is used.
ADD_TEST(NAME "scripts/bench_compare"
  COMMAND "${CMAKE_COMMAND}"
    -D "ZORBA_EXE=${ZORBA_EXE}"
    -D "CXX=${CMAKE_CXX_COMPILER}"
    -D "SRC_DIR=${CMAKE_SOURCE_DIR}"
    -D "WORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/bench_compare"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/bench_compare.cmake")
SET_TESTS_PROPERTIES("scripts/bench_compare" PROPERTIES PASS_REGULAR_EXPRESSION "Passed!")

# The expressions of an index declaration are optimized with hoisting and index
# joins, whatever the query that imports the declaration does.
ADD_TEST(NAME "scripts/index_decl_hoisting"
  COMMAND "${CMAKE_COMMAND}"
    -D "ZORBA_EXE=${ZORBA_EXE}"
    -D "SRC_DIR=${CMAKE_SOURCE_DIR}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/index_decl_hoisting.cmake")
SET_TESTS_PROPERTIES("scripts/index_decl_hoisting" PROPERTIES PASS_REGULAR_EXPRESSION "Passed!")
