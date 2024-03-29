/*
 * Copyright 2006-2016 zorba.io
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This header is configured by Zorba's build process -- DO NOT EDIT!

#ifndef ZORBA_CONFIG_H
#define ZORBA_CONFIG_H

////////// Platform & Compiler ////////////////////////////////////////////////

// Platform
#cmakedefine APPLE
#cmakedefine LINUX
#cmakedefine MINGW
#cmakedefine UNIX
#ifndef WIN32
#cmakedefine WIN32
#endif
#cmakedefine CYGWIN
#ifdef CYGWIN
# undef WIN32
#endif /* CYGWIN */
#cmakedefine ZORBA_BIG_ENDIAN @ZORBA_BIG_ENDIAN@
#cmakedefine ZORBA_LITTLE_ENDIAN @ZORBA_LITTLE_ENDIAN@

#ifdef WIN32
# pragma warning( disable: 4251 )
#endif /* WIN32 */

// Platform headers
#cmakedefine ZORBA_HAVE_COLL_H
#cmakedefine ZORBA_HAVE_EXECINFO_H
#cmakedefine ZORBA_HAVE_FLEXLEXER_H
#cmakedefine ZORBA_HAVE_ICONV_H
#cmakedefine ZORBA_HAVE_INTTYPES_H
#cmakedefine ZORBA_HAVE_LIMITS_H
#cmakedefine ZORBA_HAVE_PTHREAD_H
#cmakedefine ZORBA_HAVE_STDINT_H
#cmakedefine ZORBA_HAVE_STDLIB_H
#cmakedefine ZORBA_HAVE_SYS_MOUNT_H
#cmakedefine ZORBA_HAVE_SYS_TYPES_H
#cmakedefine ZORBA_HAVE_TZFILE_H
#cmakedefine ZORBA_HAVE_USTRING_H
#cmakedefine ZORBA_HAVE_UTYPES_H
#cmakedefine ZORBA_HAVE_UUID_H

// Platform functions
#cmakedefine ZORBA_HAVE_CLOCKGETTIME
#cmakedefine ZORBA_HAVE_GETRUSAGE
#cmakedefine ZORBA_HAVE_STRCAT_S_FUNCTION
#cmakedefine ZORBA_HAVE_STRCPY_S_FUNCTION
#cmakedefine ZORBA_HAVE_STRICMP_FUNCTION
#cmakedefine ZORBA_HAVE_STRTOF_FUNCTION
#cmakedefine ZORBA_HAVE_STRTOLL_FUNCTION

// Platform types
#cmakedefine ZORBA_HAVE_INT32_T
#cmakedefine ZORBA_HAVE_INT64_T
#cmakedefine ZORBA_HAVE_MS_INT32
#cmakedefine ZORBA_HAVE_MS_UINT32
#cmakedefine ZORBA_HAVE_UINT32_T
#cmakedefine ZORBA_HAVE_STRUCT_TM_TM_GMTOFF
#cmakedefine ZORBA_HAVE_STRUCT_TM___TM_GMTOFF

// Platform type sizes
#cmakedefine ZORBA_SIZEOF_DOUBLE        @ZORBA_SIZEOF_DOUBLE@
#cmakedefine ZORBA_SIZEOF_FLOAT         @ZORBA_SIZEOF_FLOAT@
#cmakedefine ZORBA_SIZEOF_INT           @ZORBA_SIZEOF_INT@
#cmakedefine ZORBA_SIZEOF_LONG          @ZORBA_SIZEOF_LONG@
#cmakedefine ZORBA_SIZEOF_LONG_LONG     @ZORBA_SIZEOF_LONG_LONG@
#cmakedefine ZORBA_SIZEOF_POINTER       @ZORBA_SIZEOF_POINTER@
#cmakedefine ZORBA_SIZEOF_SHORT         @ZORBA_SIZEOF_SHORT@
#cmakedefine ZORBA_SIZEOF_SIZE_T        @ZORBA_SIZEOF_SIZE_T@
#cmakedefine ZORBA_SIZEOF_WCHAR_T       @ZORBA_SIZEOF_WCHAR_T@

// Platform libraries
#cmakedefine ZORBA_HAVE_CURL

#ifdef ZORBA_HAVE_STDINT_H
# include <stdint.h>
#endif

#ifdef ZORBA_HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#ifndef ZORBA_HAVE_INT32_T
# ifdef ZORBA_HAVE_MS_INT32
    typedef __int32 int32_t;
# endif
#endif

#ifndef ZORBA_HAVE_UINT32_T
# ifdef ZORBA_HAVE_MS_UINT32
    typedef unsigned __int32 uint32_t;
# endif
#endif /* ZORBA_HAVE_UINT32_T */

#ifndef ZORBA_HAVE_INT64_T
typedef __int64 int64_t;
#endif /* ZORBA_HAVE_INT64_T */

// Compiler
#cmakedefine CLANG
#cmakedefine MSVC
#cmakedefine MSVC60
#cmakedefine MSVC70
#cmakedefine MSVC71
#cmakedefine MSVC80

// C++11 language features
#cmakedefine ZORBA_CXX_NULLPTR
#cmakedefine ZORBA_CXX_STATIC_ASSERT

// C++11 types
#cmakedefine ZORBA_HAVE_ENABLE_IF
#cmakedefine ZORBA_HAVE_IS_SAME
#cmakedefine ZORBA_HAVE_UNIQUE_PTR
#cmakedefine ZORBA_HAVE_UNORDERED_MAP
#cmakedefine ZORBA_HAVE_UNORDERED_SET

////////// C++ tr1 include directory & namespace //////////////////////////////

#cmakedefine ZORBA_TR1_IN_TR1_SUBDIRECTORY
#cmakedefine ZORBA_TR1_NS_IS_STD_TR1

#ifdef ZORBA_TR1_NS_IS_STD_TR1
# define ZORBA_TR1_NS std::tr1
#else
# define ZORBA_TR1_NS std
#endif

////////// GCC diagnostics ////////////////////////////////////////////////////

#if defined( __GNUC__ ) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 460)
# define GCC_GREATER_EQUAL_460 1
#endif

#if defined( GCC_GREATER_EQUAL_460 ) || defined( __llvm__ )
# define GCC_PRAGMA_DIAGNOSTIC_PUSH 1
#endif

////////// Zorba //////////////////////////////////////////////////////////////

#cmakedefine ZORBA_STORE_NAME "zorba_${ZORBA_STORE_NAME}"

// Version
#define ZORBA_VERSION "${ZORBA_VERSION}"
#define ZORBA_MAJOR_NUMBER ${ZORBA_MAJOR_NUMBER}
#define ZORBA_MINOR_NUMBER ${ZORBA_MINOR_NUMBER}
#define ZORBA_PATCH_NUMBER ${ZORBA_PATCH_NUMBER}
#define ZORBA_CORE_URI_DIR "${ZORBA_CORE_URI_DIR}"
#define ZORBA_CORE_LIB_DIR "${ZORBA_CORE_LIB_DIR}"
#define ZORBA_NONCORE_URI_DIR "${ZORBA_NONCORE_URI_DIR}"
#define ZORBA_NONCORE_LIB_DIR "${ZORBA_NONCORE_LIB_DIR}"

// Zorba features
#cmakedefine ZORBA_NO_FULL_TEXT
#cmakedefine ZORBA_NO_ICU
#cmakedefine ZORBA_NO_XMLSCHEMA
#cmakedefine ZORBA_NUMERIC_OPTIMIZATION
#cmakedefine ZORBA_VERIFY_PEER_SSL_CERTIFICATE
#cmakedefine ZORBA_WITH_BIG_INTEGER
#cmakedefine ZORBA_WITH_DEBUGGER
#cmakedefine ZORBA_WITH_FILE_ACCESS
#cmakedefine ZORBA_WITH_LIBXML2_SAX

// Zorba parser configuration
#cmakedefine ZORBA_DEBUG_PARSER 
#cmakedefine ZORBA_DEBUG_STRING 

// Zorba runtime configuration parameters
#define ZORBA_FLOAT_POINT_PRECISION ${ZORBA_FLOAT_POINT_PRECISION}

// Zorba threading mechanism
#cmakedefine ZORBA_FOR_ONE_THREAD_ONLY     
#cmakedefine ZORBA_HAVE_PTHREAD_SPINLOCK
#cmakedefine ZORBA_HAVE_PTHREAD_MUTEX

// XQueryX
#cmakedefine ZORBA_XQUERYX

////////// Building Zorba /////////////////////////////////////////////////////

#ifndef BUILDING_ZORBA_STATIC
# if defined WIN32 || defined CYGWIN
#   ifdef zorba_EXPORTS
#     ifdef __GNUC__
#       define ZORBA_DLL_PUBLIC __attribute__((dllexport))
#     else
#       define ZORBA_DLL_PUBLIC __declspec(dllexport)
#     endif /* __GNUC__ */
#   else /* zorba_EXPORTS */
#     ifdef __GNUC__
#       define ZORBA_DLL_PUBLIC __attribute__((dllimport))
#     else
#       define ZORBA_DLL_PUBLIC __declspec(dllimport)
#     endif /* __GNUC__ */
#   endif /* zorba_EXPORTS */
#   define ZORBA_DLL_LOCAL
# else
#   if __GNUC__ >= 4
#     define ZORBA_DLL_PUBLIC __attribute__ ((visibility("default")))
#     define ZORBA_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#   else
#     define ZORBA_DLL_PUBLIC
#     define ZORBA_DLL_LOCAL
#   endif
# endif
#else /* BUILDING_ZORBA_STATIC */
# define ZORBA_DLL_PUBLIC
# define ZORBA_DLL_LOCAL
#endif /* BUILDING_ZORBA_STATIC */

#endif /* ZORBA_CONFIG_H */
/* vim:set et sw=2 ts=2: */
