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

INCLUDE_DIRECTORIES(AFTER ${CMAKE_SOURCE_DIR}/src/)
INCLUDE_DIRECTORIES(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE("${CMAKE_SOURCE_DIR}/cmake_modules/ZorbaGenerateExe.cmake")

IF (ZORBA_WITH_DEBUGGER)
  IF (NOT WIN32)
    FIND_PACKAGE (Libedit)
    IF (LIBEDIT_FOUND)
      INCLUDE_DIRECTORIES (${LIBEDIT_INCLUDE_DIRS})
      SET (REQUIRED_LIBS ${LIBEDIT_LIBRARIES})
      SET (ZORBA_HAVE_LIBEDIT_H ${LIBEDIT_FOUND})
    ENDIF (LIBEDIT_FOUND)
  ENDIF (NOT WIN32)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/debugger/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/debugger/config.h)
  MESSAGE(STATUS "configured ${CMAKE_CURRENT_SOURCE_DIR}/debugger/config.h.cmake --> ${CMAKE_CURRENT_BINARY_DIR}/debugger/config.h")
  INCLUDE_DIRECTORIES (BEFORE ${CMAKE_CURRENT_BINARY_DIR}/debugger)

  SET (DEBUG_CLIENT_SRCS
    debugger/main.cpp
    debugger/xqdb_client.cpp
    debugger/process_listener.cpp
    debugger/command_prompt.cpp
    debugger/command_line_handler.cpp
    debugger/event_handler.cpp
  )
  
  IF(ZORBA_HAVE_PTHREAD_H)
    SET(REQUIRED_LIBS ${REQUIRED_LIBS} pthread)
  ENDIF(ZORBA_HAVE_PTHREAD_H)  
  
  ZORBA_GENERATE_EXE ("xqdb" "${DEBUG_CLIENT_SRCS}" "${REQUIRED_LIBS}" "xqdb" "bin")
ENDIF (ZORBA_WITH_DEBUGGER)

SET(SRCS
  zorbacmd_args.cpp
  zorbacmd_props.cpp
  timers.cpp
  zorbacmd.cpp
)

ZORBA_GENERATE_EXE("zorbacmd" "${SRCS}" "" "zorba" "bin")

IF(UNIX)
  ADD_CUSTOM_TARGET(zorbacmd_man help2man -N -s1 -n Zorba -S "zorba.io" ./zorba > "${PROJECT_BINARY_DIR}/doc/zorba.1")
  ADD_DEPENDENCIES(zorbacmd_man zorbacmd)
ENDIF(UNIX)

# test the basic features of the command line tool
ZORBA_ADD_TEST(bin/zorba_f1 zorbacmd blub.xq)
ZORBA_SET_TEST_PROPERTY(bin/zorba_f1 PASS_REGULAR_EXPRESSION ".*file {blub.xq} not found or not readable.*")

ZORBA_ADD_TEST(bin/zorba1 zorbacmd --omit-xml-declaration -q "1+1")
ZORBA_SET_TEST_PROPERTY(bin/zorba1 PASS_REGULAR_EXPRESSION "2")

ZORBA_ADD_TEST(bin/zorba2 zorbacmd --serialize-html -q "1+1")
ZORBA_SET_TEST_PROPERTY(bin/zorba2 PASS_REGULAR_EXPRESSION "2")

ZORBA_ADD_TEST(bin/zorba3 zorbacmd -m 0 -q "1+1")
ZORBA_SET_TEST_PROPERTY(bin/zorba3 PASS_REGULAR_EXPRESSION "")

ZORBA_ADD_TEST(bin/zorba4 zorbacmd --context-item=${CMAKE_CURRENT_SOURCE_DIR}/test.xml  -m 3 -q "./test/child[1]")
ZORBA_SET_TEST_PROPERTY(bin/zorba4 PASS_REGULAR_EXPRESSION "(.*<child>child1</child>)*")

ZORBA_ADD_TEST(bin/zorba5 zorbacmd -q -f blub.xq)
ZORBA_SET_TEST_PROPERTY(bin/zorba5 PASS_REGULAR_EXPRESSION ".*No queries submitted.*")

# --timeout and --max-udf-call-depth set the security policy limits.
ZORBA_ADD_TEST(bin/zorba_timeout zorbacmd --timeout 1 -q "count((1 to 9223372036854775807)[. lt 0])")
ZORBA_SET_TEST_PROPERTY(bin/zorba_timeout PASS_REGULAR_EXPRESSION ".*ZXQP0068.*")
ZORBA_ADD_TEST(bin/zorba_max_call_depth zorbacmd --max-udf-call-depth 10 -q "let $f := function($f, $n) { if ($n le 0) then 0 else $f($f, $n - 1) } return $f($f, 20)")
ZORBA_SET_TEST_PROPERTY(bin/zorba_max_call_depth PASS_REGULAR_EXPRESSION ".*ZXQP0069.*")

# Security policy flags: --secure, --allow, --deny, --read-path, --write-path,
# --network-prefix, --private-addresses. Each capability is shown denied and
# permitted. The queries in test/security print a marker on success; denials
# raise zerr:ZXQP0029 (zerr:ZXQP0067 for a module import). The network tests
# use 127.0.0.1:9, where nothing listens: they need no server, and an access
# the policy permits fails with another error than ZXQP0029.
SET (_secdir "${CMAKE_CURRENT_SOURCE_DIR}/test/security")
SET (_secwrite "${CMAKE_CURRENT_BINARY_DIR}/security-write")
FILE (MAKE_DIRECTORY "${_secwrite}")

MACRO (ZORBA_SECURITY_TEST NAME PASS)
  ZORBA_ADD_TEST(bin/zorba_security_${NAME} zorbacmd ${ARGN})
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_${NAME}
    PASS_REGULAR_EXPRESSION "${PASS}")
ENDMACRO (ZORBA_SECURITY_TEST)

# file-read, --read-path
ZORBA_SECURITY_TEST(read_default "cli-doc"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_deny "ZXQP0029"
  --deny file-read -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_secure "ZXQP0029"
  --secure -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_secure_allow "cli-doc"
  --secure --allow file-read -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_deny_all_allow "cli-doc"
  --deny all --allow file-read -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_path_in "cli-doc"
  --secure --allow file-read --read-path "${_secdir}/data"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_path_out "ZXQP0029"
  --read-path "${_secdir}/other"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_path_dotdot "ZXQP0029"
  --read-path "${_secdir}/data/sub"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}/data/sub/../..")
ZORBA_SECURITY_TEST(read_path_warning "warning: --read-path has no effect"
  --secure --read-path "${_secdir}/data"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")

# external-entities
ZORBA_SECURITY_TEST(xxe_default "cli-secret"
  -f -q "${_secdir}/xxe.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(xxe_deny "FODC0006"
  --deny external-entities -f -q "${_secdir}/xxe.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(xxe_secure "FODC0006"
  --secure -f -q "${_secdir}/xxe.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(xxe_secure_allow "cli-secret"
  --secure --allow external-entities,file-read
  -f -q "${_secdir}/xxe.xq" -e "dir:=${_secdir}")

# environment
ZORBA_SECURITY_TEST(env_default "env:true:true"
  -f -q "${_secdir}/env.xq")
ZORBA_SECURITY_TEST(env_deny "env:false:false"
  --deny environment -f -q "${_secdir}/env.xq")
ZORBA_SECURITY_TEST(env_secure "env:false:false"
  --secure -f -q "${_secdir}/env.xq")
ZORBA_SECURITY_TEST(env_secure_allow "env:true:true"
  --secure --allow environment -f -q "${_secdir}/env.xq")

# file-write, --write-path, native-modules (the file module is native)
ZORBA_SECURITY_TEST(write_default "written"
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/default.txt")
ZORBA_SECURITY_TEST(write_deny "ZXQP0029"
  --deny file-write
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/deny.txt")
ZORBA_SECURITY_TEST(write_secure "ZXQP0067"
  --secure -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/secure.txt")
ZORBA_SECURITY_TEST(write_no_native "ZXQP0067"
  --deny native-modules
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/no_native.txt")
ZORBA_SECURITY_TEST(write_path_in "written"
  --secure --allow file-write,native-modules --write-path "${_secwrite}"
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/in.txt")
ZORBA_SECURITY_TEST(write_path_out "ZXQP0029"
  --secure --allow file-write,native-modules --write-path "${_secwrite}/sub"
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/out.txt")

# network, --network-prefix, --private-addresses
ZORBA_SECURITY_TEST(network_deny "error:ZXQP0029"
  --deny network
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_private "error:ZXQP0029"
  --secure --allow network,native-modules
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_private_deny "error:ZXQP0029"
  --private-addresses deny
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_private_allow "error:"
  --secure --allow network,native-modules --private-addresses allow
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_network_private_allow
  FAIL_REGULAR_EXPRESSION "ZXQP0029")
ZORBA_SECURITY_TEST(network_prefix_out "error:ZXQP0029"
  --network-prefix "http://127.0.0.1:9/allowed/"
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_prefix_in "error:"
  --secure --allow network,native-modules
  --network-prefix "http://127.0.0.1:9/allowed/"
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/allowed/doc.xml")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_network_prefix_in
  FAIL_REGULAR_EXPRESSION "ZXQP0029")

# --max-call-depth is --max-udf-call-depth
ZORBA_SECURITY_TEST(max_call_depth "ZXQP0069"
  --max-call-depth 10 -q "let $f := function($f, $n) { if ($n le 0) then 0 else $f($f, $n - 1) } return $f($f, 20)")

# bad capability name
ZORBA_SECURITY_TEST(bad_capability "comma-separated list of"
  --deny file-read,bogus -q "1")

IF (UNIX)
  ADD_TEST (NAME bin/shebang
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test/shebang.xq"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
  SET_TESTS_PROPERTIES (bin/shebang PROPERTIES PASS_REGULAR_EXPRESSION "shebang success")
ENDIF (UNIX)

# test the compile-only option of the command line tool which is used for semantic checking in eclipse
ZORBA_ADD_TEST(bin/zorba_compilechk1 zorbacmd -q "${CMAKE_CURRENT_SOURCE_DIR}/test/mymod.xq" -f -l)
ZORBA_SET_TEST_PROPERTY(bin/zorba_compilechk1 PASS_REGULAR_EXPRESSION ".*mymod.xq>:22,8:.*:XPST0008.*")

ZORBA_ADD_TEST(bin/zorba_compilechk2 zorbacmd -q "${CMAKE_CURRENT_SOURCE_DIR}/test/mymod.xq" -f -l -x)
ZORBA_SET_TEST_PROPERTY(bin/zorba_compilechk2 PASS_REGULAR_EXPRESSION ".*XPST0008.*mymod.xq.*line-begin=['\"]22['\"] column-begin=['\"]8['\"] line-end=['\"]22['\"] column-end=['\"]10['\"].*")

# test compile checking to work with library modules that have an invalid target namespace uri
# test for bug #2934414
ZORBA_ADD_TEST(bin/zorba_compilechk3 zorbacmd -q "${CMAKE_CURRENT_SOURCE_DIR}/test/mymod2.xq" -f -l -x)
ZORBA_SET_TEST_PROPERTY(bin/zorba_compilechk3 PASS_REGULAR_EXPRESSION ".*XQST0046.*mymod2.xq.*line-begin=['\"]17['\"] column-begin=['\"]1['\"] line-end=['\"]17['\"] column-end=['\"]26['\"].*")

# test the --option option to set an option in the static context
ZORBA_ADD_TEST(bin/zorba_option zorbacmd -q "${CMAKE_CURRENT_SOURCE_DIR}/test/option.xq" -f --option "{http://zorba.io/}option=value")
ZORBA_SET_TEST_PROPERTY(bin/zorba_option PASS_REGULAR_EXPRESSION "value")

# a serialization parameter declared in the query prolog applies
# (XQuery 3.0 2.2.4)
ZORBA_ADD_TEST(bin/zorba_output_method zorbacmd -q "${CMAKE_CURRENT_SOURCE_DIR}/test/output_method.xq" -f)
ZORBA_SET_TEST_PROPERTY(bin/zorba_output_method PASS_REGULAR_EXPRESSION "^x")
ZORBA_SET_TEST_PROPERTY(bin/zorba_output_method FAIL_REGULAR_EXPRESSION "<a>|error")
# ... unless the command line sets it
ZORBA_ADD_TEST(bin/zorba_output_method_cli zorbacmd -q "${CMAKE_CURRENT_SOURCE_DIR}/test/output_method.xq" -f -z method=xml)
ZORBA_SET_TEST_PROPERTY(bin/zorba_output_method_cli PASS_REGULAR_EXPRESSION "<a>x</a>")

# test the --uri-path, --lib-path, and --module-path args.
# Use the versioning tests as they utilize test modules already.
SET (_testdir
  "${CMAKE_CURRENT_SOURCE_DIR}/../test/rbkt/Queries/zorba")
SET (_uridir "${CMAKE_BINARY_DIR}/TEST_URI_PATH")
SET (_libdir "${CMAKE_BINARY_DIR}/TEST_LIB_PATH")
ZORBA_ADD_TEST(bin/zorba_uri_path_1 zorbacmd
  -f -q "${_testdir}/versioning/import1.xq"
  --uri-path "${_uridir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_uri_path_1
  PASS_REGULAR_EXPRESSION "hello, v2")

ZORBA_ADD_TEST(bin/zorba_uri_path_2 zorbacmd
  -f -q "${_testdir}/versioning/external1.xq"
  --uri-path "${_uridir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_uri_path_2
  PASS_REGULAR_EXPRESSION "ZXQP0008")

ZORBA_ADD_TEST(bin/zorba_uri_path_3 zorbacmd
  -f -q "${_testdir}/versioning/external1.xq"
  --uri-path "${_uridir}" --lib-path "${_libdir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_uri_path_3
  PASS_REGULAR_EXPRESSION "hello, v2")

# test the --disable-http-resolution flag (--deny remote-import), using the corresponding test query
# from rbkt/zorba/uris/disable-http and a timeout.
# (the test query imports a schema)
IF(NOT ZORBA_NO_XMLSCHEMA)
  ZORBA_ADD_TEST(bin/zorba_disable_http_download zorbacmd
  -f -q "${_testdir}/uris/disable-http.xq" --disable-http-resolution)
  ZORBA_SET_TEST_PROPERTY(bin/zorba_disable_http_download
  PASS_REGULAR_EXPRESSION "ZXQP0029")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_disable_http_download TIMEOUT 5)
ENDIF(NOT ZORBA_NO_XMLSCHEMA)

IF(WIN32)
  SET(PATH_SEP "\;")
ELSE(WIN32)
  SET(PATH_SEP ":")
ENDIF(WIN32)
ZORBA_ADD_TEST(bin/zorba_module_path_1 zorbacmd
  -f -q "${_testdir}/versioning/external1.xq"
  --module-path "${_libdir}${PATH_SEP}${_uridir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_module_path_1
  PASS_REGULAR_EXPRESSION "hello, v2")

# test compilation of the perf script
ZORBA_ADD_TEST(bin/zorba_compilechk4 zorbacmd 
  -q "${CMAKE_CURRENT_SOURCE_DIR}/../test/zperf/src/start.xq" -f --compile-only)
ZORBA_SET_TEST_PROPERTY(bin/zorba_compilechk4
  PASS_REGULAR_EXPRESSION "[]|[XQST0059]")
