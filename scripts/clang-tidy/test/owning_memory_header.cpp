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

// Includes owning_memory_header.h, the header fixture. Nothing in this file
// is reported.

#include "owning_memory_header.h"

namespace fixture {

void use_header() {
  header_release(header_make_raw());
  zorba::rchandle<HeaderNode> h = header_make_handle();
  (void)h;
}

} // namespace fixture
