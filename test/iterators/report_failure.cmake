# Copyright 2026 zorba.io
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

# Prints why a zorba command of an iterator test failed: its exit status, its
# stderr and, when a signal ended it (EXECUTE_PROCESS then gives the signal's
# name instead of a number), the stacks of the same command run again under gdb.
FUNCTION (REPORT_FAILURE what exit_code err_file)
  MESSAGE (STATUS "${what} failed. Exit code: ${exit_code}.")
  IF (EXISTS "${err_file}")
    FILE (READ "${err_file}" _stderr)
    MESSAGE (STATUS "stderr:\n${_stderr}")
  ENDIF ()
  IF ("${exit_code}" MATCHES "^[0-9]+$")
    RETURN ()
  ENDIF ()
  FIND_PROGRAM (_gdb gdb)
  IF (NOT _gdb)
    MESSAGE (STATUS "No gdb: no stack.")
    RETURN ()
  ENDIF ()
  EXECUTE_PROCESS (
    COMMAND ${_gdb} -q -batch -ex "run > /dev/null"
      -ex "thread apply all bt" --args ${ARGN}
    OUTPUT_VARIABLE _stack
    ERROR_VARIABLE _stack)
  MESSAGE (STATUS "The command again, under gdb:\n${_stack}")
ENDFUNCTION ()
