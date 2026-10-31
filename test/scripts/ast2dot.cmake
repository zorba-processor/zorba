# Runs scripts/ast2dot.xq on the AST of ast2dot_query.xq and checks the DOT.
# Expects ZORBA_EXE, SRC_DIR (repo root) and WORK_DIR.

FILE(MAKE_DIRECTORY "${WORK_DIR}")

EXECUTE_PROCESS(
  COMMAND "${ZORBA_EXE}" --no-logo --parse-only --print-ast
          -q "${SRC_DIR}/test/scripts/ast2dot_query.xq"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err)
IF(NOT rc EQUAL 0)
  MESSAGE(FATAL_ERROR "zorba --print-ast failed (${rc}): ${err}")
ENDIF()

# drop the "AST for <file>" header line
STRING(REGEX REPLACE "(^|\n)AST for [^\n]*" "\\1" out "${out}")
FILE(WRITE "${WORK_DIR}/ast.xml" "${out}")

EXECUTE_PROCESS(
  COMMAND "${ZORBA_EXE}" --no-logo -z method=text
          -q "${SRC_DIR}/scripts/ast2dot.xq"
          -e "ast=${WORK_DIR}/ast.xml"
  RESULT_VARIABLE rc
  OUTPUT_VARIABLE dot
  ERROR_VARIABLE err)
IF(NOT rc EQUAL 0)
  MESSAGE(FATAL_ERROR "ast2dot.xq failed (${rc}): ${err}\nAST was:\n${out}")
ENDIF()
FILE(WRITE "${WORK_DIR}/ast.dot" "${dot}")

FOREACH(needle
    "digraph ast {"
    "label=\"ParseNodeTree\""
    "label=\"FLWORExpr\""
    "label=\"VarDecl"
    "FunctionCall"
    "label=\"NumericLiteral"
    " -> n_")
  STRING(FIND "${dot}" "${needle}" pos)
  IF(pos EQUAL -1)
    MESSAGE(FATAL_ERROR "DOT output lacks '${needle}':\n${dot}")
  ENDIF()
ENDFOREACH()
# pos/ptr must not leak into labels
IF(dot MATCHES "ptr=")
  MESSAGE(FATAL_ERROR "ptr attribute leaked into DOT:\n${dot}")
ENDIF()
MESSAGE("Passed!")
