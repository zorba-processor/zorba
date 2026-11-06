# Runs ZORBACMD with ARGS (separated by "|"), prints what it wrote, and fails
# unless it exits with status 0 and its output matches PATTERN: a plan that is
# printed before the query crashes or raises an error does not pass.
string(REPLACE "|" ";" _args "${ARGS}")
execute_process(COMMAND "${ZORBACMD}" ${_args}
                RESULT_VARIABLE _status
                OUTPUT_VARIABLE _out
                ERROR_VARIABLE _err)
message("${_out}")
if (NOT _err STREQUAL "")
  message("stderr:\n${_err}")
endif ()
if (NOT _status STREQUAL "0")
  message(FATAL_ERROR "exit status: ${_status}")
endif ()
if (NOT _out MATCHES "${PATTERN}")
  message(FATAL_ERROR "the output does not match: ${PATTERN}")
endif ()
