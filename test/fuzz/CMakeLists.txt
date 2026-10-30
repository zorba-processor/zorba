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


# libFuzzer targets, configured only when ZORBA_BUILD_FUZZERS is ON (the
# top-level CMakeLists.txt sets the instrumentation flags). Local use:
#   CC=clang CXX=clang++ cmake -G Ninja -DZORBA_BUILD_FUZZERS=ON ... && ninja fuzzers
#   build/test/fuzz/xquery_compile -dict=test/fuzz/xquery.dict corpus/
# CI: .clusterfuzzlite/build.sh and .github/workflows/cflite_*.yml.

SET(ZORBA_FUZZ_TARGETS xquery_compile jsoniq_compile xml_parse json_parse)

SEPARATE_ARGUMENTS(_zorba_fuzz_engine UNIX_COMMAND "${ZORBA_FUZZING_ENGINE}")

FOREACH(target ${ZORBA_FUZZ_TARGETS})
  ADD_EXECUTABLE(${target} ${target}.cpp fuzz_common.h)
  TARGET_LINK_LIBRARIES(${target} zorba_${ZORBA_STORE_NAME})
  TARGET_LINK_OPTIONS(${target} PRIVATE ${_zorba_fuzz_engine})
  SET_TARGET_PROPERTIES(${target} PROPERTIES FOLDER "Fuzz")
ENDFOREACH(target)

ADD_CUSTOM_TARGET(fuzzers DEPENDS ${ZORBA_FUZZ_TARGETS})

# Regression inputs: regressions/<target>/<name> must run cleanly through
# <target> (one input per crash fixed in the core).
FOREACH(target ${ZORBA_FUZZ_TARGETS})
  FILE(GLOB _inputs "${CMAKE_CURRENT_SOURCE_DIR}/regressions/${target}/*")
  FOREACH(input ${_inputs})
    GET_FILENAME_COMPONENT(_name ${input} NAME)
    ADD_TEST(NAME "fuzz/${target}/${_name}" COMMAND ${target} ${input})
  ENDFOREACH(input)
ENDFOREACH(target)
