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

# Runs clang-tidy with the Zorba plugin and the check CHECK on FIXTURE and
# compares the reported lines with the lines of FIXTURE that carry the
# comment "// VIOLATION". The test passes when both sets are equal, clang-tidy
# exits non-zero if and only if something is reported, and the fixture
# compiles.
#
# DIFF sets the check's DiffFile option (a "git diff -U0" output); EXPECTED
# (a list of line numbers, "-" for none) then replaces the marked lines as
# the expected set. MARKERS names the file whose lines are compared (default
# FIXTURE; a header the fixture includes) and HEADER_FILTER the
# HeaderFilterRegex (default: none; CI uses '.*').
#
#   cmake -DCLANG_TIDY=<clang-tidy> -DPLUGIN=<module> -DCHECK=<name>
#         -DFIXTURE=<file.cpp> [-DSYSROOT=<sysroot>]
#         [-DDIFF=<file.diff>] [-DEXPECTED=<line,...>]
#         [-DMARKERS=<file>] [-DHEADER_FILTER=<regex>]
#         -P run_fixture.cmake

foreach(var CLANG_TIDY PLUGIN CHECK FIXTURE)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "run_fixture.cmake: ${var} is not set")
  endif()
endforeach()

get_filename_component(fixture_dir "${FIXTURE}" DIRECTORY)
if(NOT MARKERS)
  set(MARKERS "${FIXTURE}")
endif()
get_filename_component(fixture_name "${MARKERS}" NAME)

set(extra_args)
if(SYSROOT)
  set(extra_args -isysroot "${SYSROOT}")
endif()

set(check_options "")
if(DIFF)
  set(check_options ", CheckOptions: {${CHECK}.DiffFile: '${DIFF}'}")
endif()

execute_process(
  COMMAND "${CLANG_TIDY}" "-load=${PLUGIN}"
    "--config={Checks: '-*,${CHECK}', WarningsAsErrors: '*', HeaderFilterRegex: '${HEADER_FILTER}'${check_options}}"
    "${FIXTURE}" -- -std=c++23 "-I${fixture_dir}" ${extra_args}
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err)
message("${out}${err}")

if(out MATCHES "clang-diagnostic-error" OR err MATCHES "clang-diagnostic-error")
  message(FATAL_ERROR "${fixture_name}: the fixture does not compile")
endif()

# Reported lines.
set(reported)
string(REPLACE "\n" ";" out_lines "${out}")
foreach(line IN LISTS out_lines)
  if(line MATCHES "${fixture_name}:([0-9]+):[0-9]+: (warning|error): .*\\[${CHECK}(,|\\])")
    list(APPEND reported "${CMAKE_MATCH_1}")
  endif()
endforeach()
list(REMOVE_DUPLICATES reported)
list(SORT reported COMPARE NATURAL)

# Expected lines.
set(expected)
if(DEFINED EXPECTED)
  string(REPLACE "," ";" expected "${EXPECTED}")
  list(REMOVE_ITEM expected "-")
else()
  file(STRINGS "${MARKERS}" fixture_lines)
  set(n 0)
  foreach(line IN LISTS fixture_lines)
    math(EXPR n "${n} + 1")
    if(line MATCHES "// VIOLATION$")
      list(APPEND expected "${n}")
    endif()
  endforeach()
endif()
list(SORT expected COMPARE NATURAL)

set(missing ${expected})
if(reported)
  list(REMOVE_ITEM missing ${reported})
endif()
set(unexpected ${reported})
if(expected)
  list(REMOVE_ITEM unexpected ${expected})
endif()

list(LENGTH expected n_expected)
list(LENGTH reported n_reported)
message("${fixture_name}: ${n_expected} lines expected, ${n_reported} lines reported, clang-tidy exit code ${rc}")

set(failed FALSE)
if(missing)
  message("${fixture_name}: not reported (lines): ${missing}")
  set(failed TRUE)
endif()
if(unexpected)
  message("${fixture_name}: reported but not expected (lines): ${unexpected}")
  set(failed TRUE)
endif()
if(expected AND rc EQUAL 0)
  message("${fixture_name}: clang-tidy exited 0 although findings were expected")
  set(failed TRUE)
endif()
if(NOT expected AND NOT rc EQUAL 0)
  message("${fixture_name}: clang-tidy exited ${rc} although nothing was expected")
  set(failed TRUE)
endif()
if(failed)
  message(FATAL_ERROR "${fixture_name}: FAILED")
endif()
message("${fixture_name}: PASSED")
