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


# Fuzz targets (one LLVMFuzzerTestOneInput per <target>.cpp).
#
# <target>_replay: the target linked with replay_main.cpp instead of
# libFuzzer. Always built, with any compiler, so every CI job (and the
# Sanitizers workflow, under ASan/UBSan/LSan) replays the regression inputs.
#
# <target>: the libFuzzer binary, configured only when ZORBA_BUILD_FUZZERS is
# ON (the top-level CMakeLists.txt sets the instrumentation flags). Local use:
#   CC=clang CXX=clang++ cmake -G Ninja -DZORBA_BUILD_FUZZERS=ON ... && ninja fuzzers
#   build/test/fuzz/xquery_compile -dict=test/fuzz/xquery.dict corpus/
# CI: .clusterfuzzlite/build.sh and .github/workflows/cflite_*.yml.

SET(ZORBA_FUZZ_TARGETS xquery_compile jsoniq_compile xml_parse json_parse)

FOREACH(target ${ZORBA_FUZZ_TARGETS})
  ADD_EXECUTABLE(${target}_replay ${target}.cpp replay_main.cpp fuzz_common.h)
  TARGET_LINK_LIBRARIES(${target}_replay zorba_${ZORBA_STORE_NAME})
  SET_TARGET_PROPERTIES(${target}_replay PROPERTIES FOLDER "Fuzz")

  # Regression inputs: every file in regressions/<target>/ must run cleanly
  # through <target> (one input per crash fixed in the core). The driver
  # lists the directory when the test runs, so new files need no CMake edit.
  ADD_TEST(NAME "fuzz/${target}/regressions"
           COMMAND ${target}_replay
                   "${CMAKE_CURRENT_SOURCE_DIR}/regressions/${target}")
ENDFOREACH(target)

IF(ZORBA_BUILD_FUZZERS)
  SEPARATE_ARGUMENTS(_zorba_fuzz_engine UNIX_COMMAND "${ZORBA_FUZZING_ENGINE}")
  FOREACH(target ${ZORBA_FUZZ_TARGETS})
    ADD_EXECUTABLE(${target} ${target}.cpp fuzz_common.h)
    TARGET_LINK_LIBRARIES(${target} zorba_${ZORBA_STORE_NAME})
    TARGET_LINK_OPTIONS(${target} PRIVATE ${_zorba_fuzz_engine})
    SET_TARGET_PROPERTIES(${target} PROPERTIES FOLDER "Fuzz")
  ENDFOREACH(target)
  ADD_CUSTOM_TARGET(fuzzers DEPENDS ${ZORBA_FUZZ_TARGETS})
ENDIF(ZORBA_BUILD_FUZZERS)
