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

# Installs the build into a directory, moves the installation and runs the
# installed zorba from its new place with no library path in the environment:
# the binary must find its libraries (relative RPATH). The query needs no
# module: module and library paths are fixed at configure time (see
# site/mcp.html). `-r` omits the XML declaration, so the output is exactly
# "2" with no final newline.
#
# Arguments: BUILD_DIR, WORK_DIR.

FILE (REMOVE_RECURSE "${WORK_DIR}")
EXECUTE_PROCESS (
  COMMAND "${CMAKE_COMMAND}" --install "${BUILD_DIR}" --prefix "${WORK_DIR}/original"
  RESULT_VARIABLE _rc OUTPUT_QUIET)
IF (NOT _rc EQUAL 0)
  MESSAGE (FATAL_ERROR "cmake --install failed: ${_rc}")
ENDIF ()

FILE (RENAME "${WORK_DIR}/original" "${WORK_DIR}/moved")

EXECUTE_PROCESS (
  COMMAND "${CMAKE_COMMAND}" -E env
    --unset=LD_LIBRARY_PATH --unset=DYLD_LIBRARY_PATH
    --unset=DYLD_FALLBACK_LIBRARY_PATH
    "${WORK_DIR}/moved/bin/zorba" -r -q "1 + 1"
  RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
MESSAGE (STATUS "exit ${_rc}\nstdout: [${_out}]\nstderr: ${_err}")
IF (NOT _rc EQUAL 0 OR NOT _out STREQUAL "2")
  MESSAGE (FATAL_ERROR "Relocated installation failed")
ENDIF ()
MESSAGE (STATUS "Relocated installation works")
FILE (REMOVE_RECURSE "${WORK_DIR}")
