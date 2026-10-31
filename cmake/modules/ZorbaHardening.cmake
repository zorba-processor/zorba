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

# Compiler/linker hardening. Every flag is probed, unsupported ones are skipped.
# Sanitizer builds must set ZORBA_HARDENING=OFF (FORTIFY_SOURCE conflicts with ASan).

INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckLinkerFlag)
INCLUDE(CheckCXXSourceCompiles)

IF (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND (LINUX OR APPLE))
  SET(_zorba_hardening_default ON)
ELSE()
  SET(_zorba_hardening_default OFF)
ENDIF()
OPTION(ZORBA_HARDENING
  "Enable compiler/linker hardening (FORTIFY_SOURCE, stack protector, PIE, RELRO, library assertions)"
  ${_zorba_hardening_default})

IF (ZORBA_HARDENING)
  IF (CMAKE_CXX_FLAGS MATCHES "fsanitize=address" OR
      CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE} MATCHES "fsanitize=address")
    MESSAGE(STATUS "Hardening: disabled, AddressSanitizer detected")
  ELSE()
    MESSAGE(STATUS "Hardening: enabled")

    # PIE executables, PIC libraries
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

    CHECK_CXX_COMPILER_FLAG(-fstack-protector-strong ZORBA_HAVE_STACK_PROTECTOR_STRONG)
    IF (ZORBA_HAVE_STACK_PROTECTOR_STRONG)
      ADD_COMPILE_OPTIONS(-fstack-protector-strong)
    ENDIF()

    IF (LINUX)
      CHECK_CXX_COMPILER_FLAG(-fstack-clash-protection ZORBA_HAVE_STACK_CLASH_PROTECTION)
      IF (ZORBA_HAVE_STACK_CLASH_PROTECTION)
        ADD_COMPILE_OPTIONS(-fstack-clash-protection)
      ENDIF()

      # FORTIFY_SOURCE only has an effect with optimization. Level 3 needs
      # glibc >= 2.34 and GCC >= 12 / Clang >= 17; older toolchains fall back to 2.
      SET(CMAKE_REQUIRED_FLAGS "-O2 -D_FORTIFY_SOURCE=3")
      CHECK_CXX_SOURCE_COMPILES("#include <cstring>\nint main(){char b[4];std::strcpy(b,\"a\");return b[0];}"
        ZORBA_HAVE_FORTIFY_3)
      UNSET(CMAKE_REQUIRED_FLAGS)
      IF (ZORBA_HAVE_FORTIFY_3)
        SET(_zorba_fortify 3)
      ELSE()
        SET(_zorba_fortify 2)
      ENDIF()
      ADD_COMPILE_OPTIONS(
        "$<$<NOT:$<CONFIG:Debug>>:-U_FORTIFY_SOURCE>"
        "$<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=${_zorba_fortify}>")

      CHECK_LINKER_FLAG(CXX "-Wl,-z,relro,-z,now" ZORBA_HAVE_RELRO_NOW)
      IF (ZORBA_HAVE_RELRO_NOW)
        ADD_LINK_OPTIONS("-Wl,-z,relro,-z,now")
      ENDIF()
    ENDIF()

    # Standard library assertions (bounds checks in operator[], front(), ...)
    CHECK_CXX_SOURCE_COMPILES("#include <cstddef>\n#ifndef __GLIBCXX__\n#error no\n#endif\nint main(){}"
      ZORBA_USING_LIBSTDCXX)
    CHECK_CXX_SOURCE_COMPILES("#include <cstddef>\n#ifndef _LIBCPP_VERSION\n#error no\n#endif\nint main(){}"
      ZORBA_USING_LIBCXX)
    IF (ZORBA_USING_LIBSTDCXX)
      ADD_COMPILE_DEFINITIONS(_GLIBCXX_ASSERTIONS)
    ELSEIF (ZORBA_USING_LIBCXX)
      ADD_COMPILE_DEFINITIONS(_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST)
    ENDIF()
  ENDIF()
ENDIF()
