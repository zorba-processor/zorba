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

# Zorba's clang-tidy checks, built as a clang-tidy plugin
# (clang-tidy -load=<build>/ZorbaTidyModule.so). A standalone project, not
# part of the Zorba build; .github/workflows/clang-tidy.yml builds it and runs
# its tests. It needs the LLVM and Clang development files (CMake packages
# and the clang-tidy headers) of the same version as the clang-tidy that
# loads it.
#
#   cmake -S scripts/clang-tidy -B build/clang-tidy-plugin
#   cmake --build build/clang-tidy-plugin
#   ctest --test-dir build/clang-tidy-plugin --output-on-failure

cmake_minimum_required(VERSION 3.20)
# C: LLVMConfig.cmake probes C headers.
project(zorba_clang_tidy LANGUAGES C CXX)

find_package(Clang REQUIRED CONFIG)
message(STATUS "LLVM ${LLVM_PACKAGE_VERSION} from ${LLVM_DIR}")

find_program(CLANG_TIDY clang-tidy
  HINTS "${LLVM_TOOLS_BINARY_DIR}" NO_DEFAULT_PATH)
find_program(CLANG_TIDY clang-tidy REQUIRED)
message(STATUS "clang-tidy: ${CLANG_TIDY}")

add_library(ZorbaTidyModule MODULE ZorbaTidyModule.cpp)
set_target_properties(ZorbaTidyModule PROPERTIES
  PREFIX ""
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_include_directories(ZorbaTidyModule SYSTEM PRIVATE
  ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
separate_arguments(llvm_definitions NATIVE_COMMAND "${LLVM_DEFINITIONS}")
target_compile_options(ZorbaTidyModule PRIVATE ${llvm_definitions})
if(NOT LLVM_ENABLE_RTTI)
  target_compile_options(ZorbaTidyModule PRIVATE -fno-rtti)
endif()
# The symbols of clang-tidy, Clang and LLVM come from the clang-tidy process
# that loads the plugin.
if(APPLE)
  target_link_options(ZorbaTidyModule PRIVATE -undefined dynamic_lookup)
endif()

enable_testing()
set(fixture_sysroot)
if(APPLE AND CMAKE_OSX_SYSROOT)
  set(fixture_sysroot "${CMAKE_OSX_SYSROOT}")
endif()
foreach(fixture owning_memory_violations owning_memory_allowed)
  add_test(NAME ${fixture}
    COMMAND "${CMAKE_COMMAND}"
      "-DCLANG_TIDY=${CLANG_TIDY}"
      "-DPLUGIN=$<TARGET_FILE:ZorbaTidyModule>"
      "-DCHECK=zorba-owning-memory"
      "-DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/test/${fixture}.cpp"
      "-DSYSROOT=${fixture_sysroot}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/test/run_fixture.cmake")
endforeach()
# DiffFile: only the changed lines of owning_memory_violations.diff (29 and
# 52) are reported.
add_test(NAME owning_memory_diff
  COMMAND "${CMAKE_COMMAND}"
    "-DCLANG_TIDY=${CLANG_TIDY}"
    "-DPLUGIN=$<TARGET_FILE:ZorbaTidyModule>"
    "-DCHECK=zorba-owning-memory"
    "-DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/test/owning_memory_violations.cpp"
    "-DSYSROOT=${fixture_sysroot}"
    "-DDIFF=${CMAKE_CURRENT_SOURCE_DIR}/test/owning_memory_violations.diff"
    "-DEXPECTED=29,52"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/test/run_fixture.cmake")
