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
INCLUDE(${CMAKE_SOURCE_DIR}/cmake_modules/CMakeUseBison.cmake)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake_modules/CMakeUseFlex.cmake)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake_modules/CMakeCompareVersionStrings.cmake)

SET(PARSER_SRCS
    ft_types.cpp
    symbol_table.cpp
    xquery_driver.cpp
    query_loc.cpp
    xqdoc_comment.cpp
    parser_helpers.cpp
    zorba_parser_error.cpp)

SET(PARSER_BUILD_SRCS)

###################################################################
# Parser and scanner generation
#
# The generated parsers and scanners are checked in, and the build compiles
# the checked-in copies unless ZORBA_REGENERATE_PARSER is ON. After changing
# parser.y, scanner.l or the *_includes.h files, configure a build directory
# with -DZORBA_REGENERATE_PARSER=ON, build the zorba_parser_sources target and
# run the generated copyparser.sh. CI regenerates the files in the
# docker/builder-latest image and fails if the checked-in copies differ.
OPTION(ZORBA_REGENERATE_PARSER
  "Regenerate the parsers/scanners with bison (>= 3.8) and flex (>= 2.6.4)" OFF)

SET(GENERATE_BISON_FILES FALSE)
SET(GENERATE_FLEX_FILES FALSE)

IF (ZORBA_REGENERATE_PARSER)
  COMPARE_VERSION_STRINGS("${BISON_VERSION_FULL}" "3.8" BISON_VERSION_RESULT)
  IF (NOT BISON_EXECUTABLE OR BISON_VERSION_RESULT LESS 0)
    MESSAGE(FATAL_ERROR "ZORBA_REGENERATE_PARSER needs GNU Bison >= 3.8 (found: ${BISON_VERSION_FULL})")
  ENDIF ()
  COMPARE_VERSION_STRINGS("${FLEX_VERSION_FULL}" "2.6.4" FLEX_VERSION_RESULT)
  IF (NOT FLEX_EXECUTABLE OR FLEX_VERSION_RESULT LESS 0)
    MESSAGE(FATAL_ERROR "ZORBA_REGENERATE_PARSER needs flex >= 2.6.4 (found: ${FLEX_VERSION_FULL})")
  ENDIF ()
  IF (NOT ZORBA_HAVE_FLEXLEXER_H OR NOT FLEX_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "ZORBA_REGENERATE_PARSER needs flex's FlexLexer.h")
  ENDIF ()
  SET(GENERATE_BISON_FILES TRUE)
  SET(GENERATE_FLEX_FILES TRUE)
ENDIF (ZORBA_REGENERATE_PARSER)

SET(PARSER_GENERATED_FILES)

MACRO(GENERATE_GRAMMAR_FILE PARSER_TYPE)
  SET(TARGET_PATH "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
  SET_SOURCE_FILES_PROPERTIES(${TARGET_PATH}/${PARSER_TYPE}.y PROPERTIES GENERATED TRUE)
  STRING(TOUPPER ${PARSER_TYPE} PARSER_DEFINE)
  ADD_CUSTOM_COMMAND(
      OUTPUT ${TARGET_PATH}/${PARSER_TYPE}.y
      COMMAND "printf"
      ARGS "\"%%{\\n#define" "${PARSER_DEFINE}\\n%%}\\n\">${TARGET_PATH}/${PARSER_TYPE}.y"
      COMMAND "cat"
      ARGS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/parser_y_includes.h" 
            ">>"
           "${TARGET_PATH}/${PARSER_TYPE}.y"
      COMMAND ${CMAKE_CXX_COMPILER} 
      ARGS "-xc++" "-P" "-D${PARSER_DEFINE}"
           "-E" "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/parser.y"
           ">>" "${TARGET_PATH}/${PARSER_TYPE}.y"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/parser.y" "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/parser_y_includes.h"
    )
  
  BISON_FILE(${TARGET_PATH}/${PARSER_TYPE}.y)
  LIST(APPEND PARSER_BUILD_SRCS ${TARGET_PATH}/${PARSER_TYPE}.cpp)
  LIST(APPEND PARSER_GENERATED_FILES ${TARGET_PATH}/${PARSER_TYPE}.cpp
       ${TARGET_PATH}/${PARSER_TYPE}.hpp ${TARGET_PATH}/location.hh)
ENDMACRO(GENERATE_GRAMMAR_FILE PARSER_TYPE)

IF (GENERATE_BISON_FILES)
    SET(BISON_GENERATE_DEFINES TRUE)
    IF(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
      FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
    ENDIF(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
    
    GENERATE_GRAMMAR_FILE(xquery_parser)
    GENERATE_GRAMMAR_FILE(jsoniq_parser)
ELSE (GENERATE_BISON_FILES)
    LIST(APPEND PARSER_SRCS xquery_parser.cpp jsoniq_parser.cpp)
ENDIF (GENERATE_BISON_FILES)

MACRO(GENERATE_FLEX_FILE SCANNER_TYPE)
  SET(TARGET_PATH ${CMAKE_CURRENT_BINARY_DIR}/compiler/parser)  
  SET_SOURCE_FILES_PROPERTIES(${TARGET_PATH}/${SCANNER_TYPE}.l PROPERTIES GENERATED TRUE)
  STRING(TOUPPER ${SCANNER_TYPE} SCANNER_DEFINE)
  
  ADD_CUSTOM_COMMAND(
      OUTPUT ${TARGET_PATH}/${SCANNER_TYPE}.l
      COMMAND "printf"
      ARGS "\"%%{\\n#define" "${SCANNER_DEFINE}\\n%%}\\n\">${TARGET_PATH}/${SCANNER_TYPE}.l"
      COMMAND "cat"
      ARGS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/scanner_l_includes.h" 
            ">>"
           "${TARGET_PATH}/${SCANNER_TYPE}.l"
      COMMAND ${CMAKE_CXX_COMPILER} 
      ARGS "-xc++" "-P" "-D${SCANNER_DEFINE}"
           "-E" "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/scanner.l"
           ">>" "${TARGET_PATH}/${SCANNER_TYPE}.l"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/scanner.l" "${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/scanner_l_includes.h"
    )
    
  FLEX_FILE(${TARGET_PATH}/${SCANNER_TYPE}.l)  
  LIST(APPEND PARSER_BUILD_SRCS ${CMAKE_BINARY_DIR}/src/compiler/parser/${SCANNER_TYPE}.cpp)
  LIST(APPEND PARSER_GENERATED_FILES ${CMAKE_BINARY_DIR}/src/compiler/parser/${SCANNER_TYPE}.cpp)
ENDMACRO(GENERATE_FLEX_FILE SCANNER_TYPE)

IF (GENERATE_FLEX_FILES)
    IF(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
      FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
    ENDIF(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compiler/parser")
              
    SET(FLEX_PREFIX_OUTPUTS TRUE)
    ADD_DEFINITIONS(-DFLEX_FILES_REGENERATED)
    
    GENERATE_FLEX_FILE(xquery_scanner)
    GENERATE_FLEX_FILE(jsoniq_scanner)
    LIST(APPEND PARSER_GENERATED_FILES ${CMAKE_CURRENT_BINARY_DIR}/FlexLexer.h)
ELSE (GENERATE_FLEX_FILES)
    LIST(APPEND PARSER_SRCS xquery_scanner.cpp jsoniq_scanner.cpp)
ENDIF (GENERATE_FLEX_FILES)

IF (PARSER_GENERATED_FILES)
  LIST(REMOVE_DUPLICATES PARSER_GENERATED_FILES)
  ADD_CUSTOM_TARGET(zorba_parser_sources DEPENDS ${PARSER_GENERATED_FILES})
ENDIF (PARSER_GENERATED_FILES)

# generate a script that copies the generated parser and scanner files from the build
# directory into the source directory
# this script should be used before commiting if one modified the parser/scanner
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/copyparser.sh.cmake ${CMAKE_BINARY_DIR}/copyparser.sh)
MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/copyparser.sh.cmake -->${CMAKE_BINARY_DIR}/copyparser.sh")

# The generated parsers and scanners (a few huge table-driven functions) are
# the slowest translation units of the build at -O2/-O3; -O1 compiles them
# much faster without a measurable effect on parsing time. GCC rejects the
# -O3 precompiled header for -O1 TUs (-Winvalid-pch), so they skip it.
IF (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  SET(PARSER_O1_SRCS)
  FOREACH (GENERATED_SRC xquery_parser jsoniq_parser xquery_scanner jsoniq_scanner)
    LIST(APPEND PARSER_O1_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/compiler/parser/${GENERATED_SRC}.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/compiler/parser/${GENERATED_SRC}.cpp)
  ENDFOREACH (GENERATED_SRC)
  SET_SOURCE_FILES_PROPERTIES(${PARSER_O1_SRCS} PROPERTIES
    COMPILE_OPTIONS "$<$<CONFIG:Release,RelWithDebInfo>:-O1>"
    SKIP_PRECOMPILE_HEADERS ON)
ENDIF ()
