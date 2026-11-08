# Compiles queries that import a module with an index declaration and checks the
# optimized expression trees: the expressions of the index declaration are
# optimized with hoisting and index joins whatever the query that imports it does,
# and the query keeps its own rule.
# Expects ZORBA_EXE and SRC_DIR (repo root).

SET(DIR "${SRC_DIR}/test/scripts/index_decl_hoisting")

# The tree printed after optimization for the given name, up to the next tree.
FUNCTION(optimized_tree text name result)
  STRING(FIND "${text}" "Expression tree after optimization for ${name}" start)
  IF(start EQUAL -1)
    MESSAGE(FATAL_ERROR "No optimized tree for '${name}':\n${text}")
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

FUNCTION(compile query result)
  EXECUTE_PROCESS(
    COMMAND "${ZORBA_EXE}" --no-logo --compile-only --print-optimized
            -f -q"${DIR}/${query}"
    WORKING_DIRECTORY "${DIR}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)
  IF(NOT rc EQUAL 0)
    MESSAGE(FATAL_ERROR "zorba failed on ${query} (${rc}): ${err}")
  ENDIF()
  SET(${result} "${out}\n${err}" PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(expect text needle what)
  STRING(FIND "${text}" "${needle}" pos)
  IF(pos EQUAL -1)
    MESSAGE(FATAL_ERROR "${what}: no '${needle}' in:\n${text}")
  ENDIF()
ENDFUNCTION()

FUNCTION(expect_not text needle what)
  STRING(FIND "${text}" "${needle}" pos)
  IF(NOT pos EQUAL -1)
    MESSAGE(FATAL_ERROR "${what}: '${needle}' found in:\n${text}")
  ENDIF()
ENDFUNCTION()

# A query without a call that can have a side effect.
compile(main_pure.xq pure)
optimized_tree("${pure}" "Domain expr for index" pure_index)
optimized_tree("${pure}" "main query" pure_main)
expect("${pure_index}" "probe-index-point-value" "index declaration, pure query")
expect("${pure_main}" "hoist" "pure query")

# A query with an eval: its own expressions stay in place, the index
# declaration is not affected.
compile(main_eval.xq eval)
optimized_tree("${eval}" "Domain expr for index" eval_index)
optimized_tree("${eval}" "main query" eval_main)
expect("${eval_index}" "probe-index-point-value" "index declaration, query with eval")
expect_not("${eval_main}" "hoist" "query with eval")

MESSAGE("Passed!")
