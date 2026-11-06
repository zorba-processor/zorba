# Tests bench/compare_table.xq on a sample of raw results and, on Linux,
# bench/compare.sh end to end (the same zorba as base and head).
# Expects ZORBA_EXE, CXX, SRC_DIR (repo root) and WORK_DIR.

FILE(MAKE_DIRECTORY "${WORK_DIR}")
SET(HERE "${SRC_DIR}/test/scripts/bench_compare")

FUNCTION(expect text needle)
  STRING(FIND "${text}" "${needle}" pos)
  IF(pos EQUAL -1)
    MESSAGE(FATAL_ERROR "missing '${needle}' in:\n${text}")
  ENDIF()
ENDFUNCTION()

FUNCTION(table raw out_var)
  EXECUTE_PROCESS(
    COMMAND "${ZORBA_EXE}" --no-logo --serialize-text -f
            -q "${SRC_DIR}/bench/compare_table.xq"
            -e "raw:=${raw}" -e "base:=BASE-LABEL" -e "head:=HEAD-LABEL"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)
  IF(NOT rc EQUAL 0)
    MESSAGE(FATAL_ERROR "compare_table.xq failed (${rc}): ${err}")
  ENDIF()
  SET(${out_var} "${out}" PARENT_SCOPE)
ENDFUNCTION()

# 1. The table of a sample: medians, minima, ratios, a timeout, a failed timed
# run, a different output.
table("${HERE}/sample_raw.tsv" sample)
expect("${sample}" "base: BASE-LABEL")
expect("${sample}" "head: HEAD-LABEL")
expect("${sample}" "3 queries, 2 failed, 1 with a different output.")
expect("${sample}" "| fast | 0.110 | 0.100 | 0.060 | 0.050 | 0.55x | 0.50x | 4.0 | 1.0 | 0.25x | same | ok |")
expect("${sample}" "| slowfail | 0.300 | 0.290 | - | - | - | - | 1.0 | - | - | - | **FAILED** head: timeout (run 0) |")
expect("${sample}" "| changed | 0.020 | 0.020 | 0.010 | 0.010 | 0.50x | 0.50x | 0.1 | 0.1 | 1.00x | DIFFERENT | **FAILED** head: exit-1 (run 3) |")

# 2. The driver, on Linux only (it needs GNU timeout).
IF(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  MESSAGE("driver test skipped on ${CMAKE_HOST_SYSTEM_NAME}")
  MESSAGE("Passed!")
  RETURN()
ENDIF()

EXECUTE_PROCESS(
  COMMAND "${CXX}" -std=c++23 -O1 -o "${WORK_DIR}/dirbench"
          "${SRC_DIR}/bench/directory/dirbench.cpp"
  RESULT_VARIABLE rc ERROR_VARIABLE err)
IF(NOT rc EQUAL 0)
  MESSAGE(FATAL_ERROR "cannot compile dirbench (${rc}): ${err}")
ENDIF()

FILE(REMOVE_RECURSE "${WORK_DIR}/out" "${WORK_DIR}/out-filter")
EXECUTE_PROCESS(
  COMMAND "${CMAKE_COMMAND}" -E env REPEAT=2 TIMEOUT=2 "ARGS=-e n:=41"
          sh "${SRC_DIR}/bench/compare.sh" "${ZORBA_EXE}" "${ZORBA_EXE}"
          "${WORK_DIR}/dirbench" "${HERE}/suite" "${WORK_DIR}/out"
  RESULT_VARIABLE rc OUTPUT_VARIABLE log ERROR_VARIABLE err)
IF(NOT rc EQUAL 0)
  MESSAGE(FATAL_ERROR "compare.sh failed (${rc}): ${err}\n${log}")
ENDIF()

# ok.xq and arg.jq: warm-up + 2 timed runs on both builds. fail.xq and hang.xq:
# the warm-up only; a build that failed there is not run again.
FILE(STRINGS "${WORK_DIR}/out/raw.tsv" rows)
LIST(LENGTH rows nrows)
IF(NOT nrows EQUAL 16)
  MESSAGE(FATAL_ERROR "expected 16 rows in raw.tsv, got ${nrows}:\n${rows}")
ENDIF()

table("${WORK_DIR}/out/raw.tsv" driver)
expect("${driver}" "4 queries, 2 failed, 0 with a different output.")
expect("${driver}" "| ok | ")
expect("${driver}" "| arg | ")
expect("${driver}" "| same | ok |")
expect("${driver}" "| fail | - | - | - | - | - | - | - | - | - | - |")
expect("${driver}" "**FAILED** base: exit-")
expect("${driver}" "**FAILED** base: timeout (run 0); head: timeout (run 0)")

# The argument reached the query: arg.jq returns 42 with n := 41.
FILE(STRINGS "${WORK_DIR}/out/raw.tsv" argrows REGEX "^arg.")
LIST(LENGTH argrows nargrows)
IF(NOT nargrows EQUAL 6)
  MESSAGE(FATAL_ERROR "arg.jq: expected 6 rows, got ${nargrows}")
ENDIF()

# FILTER selects by name (ok and arg; arg fails in its warm-up here, with no ARGS:
# 4 + 2 rows); one that selects nothing is an error.
EXECUTE_PROCESS(
  COMMAND "${CMAKE_COMMAND}" -E env REPEAT=1 TIMEOUT=2 "FILTER=ok,a*"
          sh "${SRC_DIR}/bench/compare.sh" "${ZORBA_EXE}" "${ZORBA_EXE}"
          "${WORK_DIR}/dirbench" "${HERE}/suite" "${WORK_DIR}/out-filter"
  RESULT_VARIABLE rc OUTPUT_VARIABLE log ERROR_VARIABLE err)
IF(NOT rc EQUAL 0)
  MESSAGE(FATAL_ERROR "compare.sh with FILTER failed (${rc}): ${err}")
ENDIF()
FILE(STRINGS "${WORK_DIR}/out-filter/raw.tsv" frows)
LIST(LENGTH frows nfrows)
IF(NOT nfrows EQUAL 6)
  MESSAGE(FATAL_ERROR "FILTER: expected 6 rows, got ${nfrows}:\n${frows}")
ENDIF()
EXECUTE_PROCESS(
  COMMAND "${CMAKE_COMMAND}" -E env FILTER=nothing-like-this
          sh "${SRC_DIR}/bench/compare.sh" "${ZORBA_EXE}" "${ZORBA_EXE}"
          "${WORK_DIR}/dirbench" "${HERE}/suite" "${WORK_DIR}/out-none"
  RESULT_VARIABLE rc OUTPUT_QUIET ERROR_QUIET)
IF(NOT rc EQUAL 2)
  MESSAGE(FATAL_ERROR "FILTER matching nothing: expected exit 2, got ${rc}")
ENDIF()

MESSAGE("Passed!")
