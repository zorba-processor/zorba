# A function annotated %an:nondeterministic stays nondeterministic when its body
# calls no nondeterministic function: a call with a loop-independent argument is
# not moved out of the loop. The same function without the annotation is.
# Expects ZORBA_EXE and SRC_DIR (repo root).

SET(DIR "${SRC_DIR}/test/scripts/udf_nondeterministic_annotation")

# The tree printed after optimization for the main query, up to the next tree.
FUNCTION(main_tree query level result)
  EXECUTE_PROCESS(
    COMMAND "${ZORBA_EXE}" --no-logo --compile-only --print-optimized
            --optimization-level ${level} -f -q "${DIR}/${query}"
    WORKING_DIRECTORY "${DIR}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)
  IF(NOT rc EQUAL 0)
    MESSAGE(FATAL_ERROR "zorba failed on ${query} at O${level} (${rc}): ${err}")
  ENDIF()
  SET(text "${out}\n${err}")
  STRING(FIND "${text}" "Expression tree after optimization for main query" start)
  IF(start EQUAL -1)
    MESSAGE(FATAL_ERROR "No optimized tree for the main query:\n${text}")
  ENDIF()
  STRING(SUBSTRING "${text}" ${start} -1 tail)
  STRING(SUBSTRING "${tail}" 1 -1 rest)
  STRING(FIND "${rest}" "Expression tree after" next)
  IF(NOT next EQUAL -1)
    MATH(EXPR next "${next} + 1")
    STRING(SUBSTRING "${tail}" 0 ${next} tail)
  ENDIF()
  SET(${result} "${tail}" PARENT_SCOPE)
ENDFUNCTION()

# The loop domain is hoisted in both queries; only a hoist of the call matters.
SET(hoisted_call "hoist#[0-9]+ \\([^)]*\\) \\[[ \n\r]*local:g")

FOREACH(level 1 2)
  main_tree(plain.xq ${level} plain)
  STRING(REGEX MATCH "${hoisted_call}" found "${plain}")
  IF(found STREQUAL "")
    MESSAGE(FATAL_ERROR "O${level}, deterministic function: call not hoisted:\n${plain}")
  ENDIF()

  main_tree(annotated.xq ${level} annotated)
  STRING(REGEX MATCH "${hoisted_call}" found "${annotated}")
  IF(NOT found STREQUAL "")
    MESSAGE(FATAL_ERROR "O${level}, %an:nondeterministic function: call hoisted:\n${annotated}")
  ENDIF()
ENDFOREACH()

MESSAGE("Passed!")
