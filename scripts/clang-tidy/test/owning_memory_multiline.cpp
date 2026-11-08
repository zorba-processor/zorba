/*
 * Copyright 2006-2016 zorba.io
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

// Fixture for zorba-owning-memory with DiffFile: a new expression is
// reported when any line of the expression is changed, not only the line
// that holds the new token. The expected lines are in CMakeLists.txt.

#include "owning_memory_mocks.h"

namespace fixture {

struct Pair {
  Pair(int a, int b) : theA(a), theB(b) {}
  int theA;
  int theB;
};

void multiline(std::vector<Pair*>& v) {
  Pair* argumentChanged = new Pair(1,
                                   2);
  Pair* beforeChanged = new Pair(3,
                                 4);
  Pair* wholeChanged = new Pair(5,
                                6);
  Pair* nextStatementChanged = new Pair(7,
                                        8);
  int changedAbove = 0;
  v.push_back(new Pair(
    9, 10));
  std::unique_ptr<Pair> smart(new Pair(11,
                                       12));
  (void)changedAbove;
}

} // namespace fixture
