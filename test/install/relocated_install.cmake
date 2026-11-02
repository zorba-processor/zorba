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
# installed zorba from its new place with no library path in the environment.
# The binary must find its libraries (relative RPATH) and its modules
# (resolved against the library's location). The installed copy of the base64
# module gets an extra function, so that the query can only succeed with that
# copy and not with the copy in the build tree.
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

FILE (GLOB_RECURSE _modules "${WORK_DIR}/moved/share/zorba/uris/*base64*")
LIST (LENGTH _modules _count)
IF (_count EQUAL 0)
  MESSAGE (FATAL_ERROR "base64 module not found in the installation")
ENDIF ()
FOREACH (_m ${_modules})
  FILE (APPEND "${_m}"
    "\ndeclare function base64:relocated() as xs:string { \"installed-copy\" };\n")
ENDFOREACH ()

EXECUTE_PROCESS (
  COMMAND "${CMAKE_COMMAND}" -E env
    --unset=LD_LIBRARY_PATH --unset=DYLD_LIBRARY_PATH
    --unset=DYLD_FALLBACK_LIBRARY_PATH
    "${WORK_DIR}/moved/bin/zorba" --omit-xml-declaration -q
    "import module namespace b = 'http://zorba.io/modules/base64'; (b:relocated(), xs:string(b:encode('hi')))"
  RESULT_VARIABLE _rc OUTPUT_VARIABLE _out ERROR_VARIABLE _err)
MESSAGE (STATUS "exit ${_rc}\nstdout: ${_out}\nstderr: ${_err}")
IF (NOT _rc EQUAL 0 OR NOT _out MATCHES "installed-copy aGk=")
  # Diagnostics: the installed modules, and the same query with the installed
  # module directory given explicitly.
  FILE (GLOB_RECURSE _all "${WORK_DIR}/moved/share/*")
  MESSAGE (STATUS "installed files under share:\n${_all}")
  EXECUTE_PROCESS (
    COMMAND "${CMAKE_COMMAND}" -E env
      --unset=LD_LIBRARY_PATH --unset=DYLD_LIBRARY_PATH
      "${WORK_DIR}/moved/bin/zorba" --omit-xml-declaration
      --uri-path "${WORK_DIR}/moved/share/zorba/uris/core" -q
      "import module namespace b = 'http://zorba.io/modules/base64'; b:relocated()"
    OUTPUT_VARIABLE _o2 ERROR_VARIABLE _e2)
  MESSAGE (STATUS "explicit uri-path: ${_o2} ${_e2}")
  MESSAGE (FATAL_ERROR "Relocated installation failed")
ENDIF ()
MESSAGE (STATUS "Relocated installation works")
FILE (REMOVE_RECURSE "${WORK_DIR}")
