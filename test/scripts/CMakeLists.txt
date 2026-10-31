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
