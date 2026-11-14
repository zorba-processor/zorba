# Bison's C++ skeleton declares noexcept two functions that allocate: the
# stack's constructor, which the parser's constructor runs, and
# basic_symbol::name(). std::bad_alloc in them would terminate the process;
# this script, run on each generated parser header, lets it reach the caller.
FILE(READ "${HEADER}" TEXT)
FOREACH (DECLARATION "stack (size_type n = 200)" "std::string name () const")
  STRING(FIND "${TEXT}" "${DECLARATION} YY_NOEXCEPT" POSITION)
  IF (POSITION EQUAL -1)
    MESSAGE(FATAL_ERROR "${HEADER}: no \"${DECLARATION} YY_NOEXCEPT\"; check the bison skeleton")
  ENDIF ()
  STRING(REPLACE "${DECLARATION} YY_NOEXCEPT" "${DECLARATION}" TEXT "${TEXT}")
ENDFOREACH ()
FILE(WRITE "${HEADER}" "${TEXT}")
