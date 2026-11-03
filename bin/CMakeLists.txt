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

INCLUDE("${CMAKE_SOURCE_DIR}/cmake/modules/ZorbaGenerateExe.cmake")

IF (ZORBA_WITH_DEBUGGER)
  FIND_PACKAGE (Libedit)
  IF (LIBEDIT_FOUND)
    INCLUDE_DIRECTORIES (${LIBEDIT_INCLUDE_DIRS})
    SET (REQUIRED_LIBS ${LIBEDIT_LIBRARIES})
    SET (ZORBA_HAVE_LIBEDIT_H ${LIBEDIT_FOUND})
  ENDIF (LIBEDIT_FOUND)

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
  directory_config.cpp
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

# -r omits the XML declaration. The exact output, "2" with no final newline,
# is checked by test/install/relocated, which compares it with STREQUAL.
ZORBA_ADD_TEST(bin/zorba_r_one_plus_one zorbacmd -r -q "1 + 1")
ZORBA_SET_TEST_PROPERTY(bin/zorba_r_one_plus_one PASS_REGULAR_EXPRESSION "^2")
ZORBA_SET_TEST_PROPERTY(bin/zorba_r_one_plus_one FAIL_REGULAR_EXPRESSION "<\\?xml")

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

# The option zorba-streaming-members, written as a literal in a parse that is
# unboxed directly, reads the members one at a time; anything else goes through
# the store as a whole.
SET (_plandir "${CMAKE_CURRENT_SOURCE_DIR}/test")
ZORBA_ADD_TEST(bin/zorba_plan_parse_default zorbacmd --print-iterator-tree -f -q "${_plandir}/plan_parse_members.jq")
ZORBA_SET_TEST_PROPERTY(bin/zorba_plan_parse_default PASS_REGULAR_EXPRESSION "MultiArrayMembersIterator")
ZORBA_SET_TEST_PROPERTY(bin/zorba_plan_parse_default FAIL_REGULAR_EXPRESSION "JSONParseMembersIterator")
FOREACH (_opt 0 1 2)
  ZORBA_ADD_TEST(bin/zorba_plan_parse_streaming_O${_opt} zorbacmd --optimization-level ${_opt} --print-iterator-tree -f -q "${_plandir}/plan_parse_streaming.jq")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_plan_parse_streaming_O${_opt} PASS_REGULAR_EXPRESSION "JSONParseMembersIterator")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_plan_parse_streaming_O${_opt} FAIL_REGULAR_EXPRESSION "MultiArrayMembersIterator")
ENDFOREACH ()
FOREACH (_q plan_parse_streaming_off plan_parse_streaming_dynamic plan_parse_streaming_strip plan_parse_array_var)
  ZORBA_ADD_TEST(bin/zorba_${_q} zorbacmd --print-iterator-tree -f -q "${_plandir}/${_q}.jq")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_${_q} PASS_REGULAR_EXPRESSION "MultiArrayMembersIterator")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_${_q} FAIL_REGULAR_EXPRESSION "JSONParseMembersIterator")
ENDFOREACH ()
ZORBA_ADD_TEST(bin/zorba_timeout_parse_members zorbacmd --timeout 1 -f -q "${_plandir}/timeout_parse_members.jq")
ZORBA_SET_TEST_PROPERTY(bin/zorba_timeout_parse_members PASS_REGULAR_EXPRESSION ".*ZXQP0068.*")
# --max-expr-depth sets the policy's expression nesting limit.
ZORBA_ADD_TEST(bin/zorba_max_expr_depth zorbacmd --max-expr-depth 10 -q "((((((((((((1))))))))))))")
ZORBA_SET_TEST_PROPERTY(bin/zorba_max_expr_depth PASS_REGULAR_EXPRESSION ".*ZXQP0070.*")

# A diagnostic hint is printed after the message; an error without one prints
# no "hint:" line. In XQuery mode $x[] stays XPST0003.
ZORBA_ADD_TEST(bin/zorba_hint_syntax zorbacmd -q "let $x := [1] return $x[]")
ZORBA_SET_TEST_PROPERTY(bin/zorba_hint_syntax PASS_REGULAR_EXPRESSION "XPST0003.*hint: .*jn:members")
ZORBA_ADD_TEST(bin/zorba_hint_prefix zorbacmd -q "math:pi()")
ZORBA_SET_TEST_PROPERTY(bin/zorba_hint_prefix PASS_REGULAR_EXPRESSION "XPST0081.*hint: .*import module namespace math")
ZORBA_ADD_TEST(bin/zorba_hint_xml zorbacmd --print-errors-as-xml -q "let $x := 3 return $x-1")
ZORBA_SET_TEST_PROPERTY(bin/zorba_hint_xml PASS_REGULAR_EXPRESSION "<hint>.*x - 1.*</hint>")
ZORBA_ADD_TEST(bin/zorba_hint_none zorbacmd -q "nosuchfunction(1)")
ZORBA_SET_TEST_PROPERTY(bin/zorba_hint_none PASS_REGULAR_EXPRESSION "XPST0017")
ZORBA_SET_TEST_PROPERTY(bin/zorba_hint_none FAIL_REGULAR_EXPRESSION "hint:")
ZORBA_ADD_TEST(bin/zorba_hint_serializer zorbacmd -z method=json -q "<a/>")
ZORBA_SET_TEST_PROPERTY(bin/zorba_hint_serializer PASS_REGULAR_EXPRESSION "JNSE0014.*hint: .*serialize")

# JNSE0012: the message says how to serialize several items as JSON.
ZORBA_ADD_TEST(bin/zorba_jnse0012 zorbacmd -z method=json -z jsoniq-multiple-items=no -q "1, 2")
ZORBA_SET_TEST_PROPERTY(bin/zorba_jnse0012 PASS_REGULAR_EXPRESSION
  "JNSE0012\\]: cannot serialize multiple top-level items as JSON; wrap them in an array or set the serialization parameter jsoniq-multiple-items to yes")

# The language of a query: its own version declaration (jsoniq version or
# xquery version) wins; else the extension of the file (.jq is JSONiq, .xq is
# XQuery); else --language (default xquery; -j, --jsoniq is --language jsoniq).
# Every query without a declaration is the one program below: it runs only in
# JSONiq ($x[] unboxes an array) and is a syntax error (XPST0003) in XQuery.
SET (_langdir "${CMAKE_CURRENT_SOURCE_DIR}/test/language")
# Sets what the output of a test must match: jsoniq (the program ran),
# xquery (it is a syntax error), or a regular expression.
MACRO (ZORBA_LANGUAGE_EXPECT NAME EXPECT)
  IF ("${EXPECT}" STREQUAL "jsoniq")
    ZORBA_SET_TEST_PROPERTY(bin/zorba_language_${NAME}
      PASS_REGULAR_EXPRESSION "1 2")
    ZORBA_SET_TEST_PROPERTY(bin/zorba_language_${NAME}
      FAIL_REGULAR_EXPRESSION "XPST0003")
  ELSEIF ("${EXPECT}" STREQUAL "xquery")
    ZORBA_SET_TEST_PROPERTY(bin/zorba_language_${NAME}
      PASS_REGULAR_EXPRESSION "XPST0003")
  ELSE ()
    ZORBA_SET_TEST_PROPERTY(bin/zorba_language_${NAME}
      PASS_REGULAR_EXPRESSION "${EXPECT}")
  ENDIF ()
ENDMACRO (ZORBA_LANGUAGE_EXPECT)


# No declaration: the extension.
ZORBA_ADD_TEST(bin/zorba_language_ext_jq zorbacmd -f -q "${_langdir}/plain.jq")
ZORBA_LANGUAGE_EXPECT(ext_jq jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_ext_xq zorbacmd -f -q "${_langdir}/plain.xq")
ZORBA_LANGUAGE_EXPECT(ext_xq xquery)
# No declaration, another extension: --language, default xquery.
ZORBA_ADD_TEST(bin/zorba_language_other_default zorbacmd -f -q "${_langdir}/plain.q")
ZORBA_LANGUAGE_EXPECT(other_default xquery)
ZORBA_ADD_TEST(bin/zorba_language_other_xquery zorbacmd --language xquery -f -q "${_langdir}/plain.q")
ZORBA_LANGUAGE_EXPECT(other_xquery xquery)
ZORBA_ADD_TEST(bin/zorba_language_other_jsoniq zorbacmd --language jsoniq -f -q "${_langdir}/plain.q")
ZORBA_LANGUAGE_EXPECT(other_jsoniq jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_other_j zorbacmd -j -f -q "${_langdir}/plain.q")
ZORBA_LANGUAGE_EXPECT(other_j jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_other_jsoniq_option zorbacmd --jsoniq -f -q "${_langdir}/plain.q")
ZORBA_LANGUAGE_EXPECT(other_jsoniq_option jsoniq)
# An inline query has no extension.
ZORBA_ADD_TEST(bin/zorba_language_inline_default zorbacmd -q "let $x := [1, 2] return $x[]")
ZORBA_LANGUAGE_EXPECT(inline_default xquery)
ZORBA_ADD_TEST(bin/zorba_language_inline_xquery zorbacmd --language xquery -q "let $x := [1, 2] return $x[]")
ZORBA_LANGUAGE_EXPECT(inline_xquery xquery)
ZORBA_ADD_TEST(bin/zorba_language_inline_jsoniq zorbacmd --language jsoniq -q "let $x := [1, 2] return $x[]")
ZORBA_LANGUAGE_EXPECT(inline_jsoniq jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_inline_j zorbacmd -j -q "let $x := [1, 2] return $x[]")
ZORBA_LANGUAGE_EXPECT(inline_j jsoniq)
# The extension wins over --language.
ZORBA_ADD_TEST(bin/zorba_language_ext_jq_xquery zorbacmd --language xquery -f -q "${_langdir}/plain.jq")
ZORBA_LANGUAGE_EXPECT(ext_jq_xquery jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_ext_xq_jsoniq zorbacmd --language jsoniq -f -q "${_langdir}/plain.xq")
ZORBA_LANGUAGE_EXPECT(ext_xq_jsoniq xquery)
ZORBA_ADD_TEST(bin/zorba_language_ext_xq_j zorbacmd -j -f -q "${_langdir}/plain.xq")
ZORBA_LANGUAGE_EXPECT(ext_xq_j xquery)
# The declaration wins over the extension and over --language.
ZORBA_ADD_TEST(bin/zorba_language_header_xquery_jq zorbacmd -f -q "${_langdir}/header_xquery.jq")
ZORBA_LANGUAGE_EXPECT(header_xquery_jq xquery)
ZORBA_ADD_TEST(bin/zorba_language_header_jsoniq_xq zorbacmd -f -q "${_langdir}/header_jsoniq.xq")
ZORBA_LANGUAGE_EXPECT(header_jsoniq_xq jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_header_xquery_option zorbacmd --language jsoniq -f -q "${_langdir}/header_xquery.q")
ZORBA_LANGUAGE_EXPECT(header_xquery_option xquery)
ZORBA_ADD_TEST(bin/zorba_language_header_jsoniq_option zorbacmd --language xquery -f -q "${_langdir}/header_jsoniq.q")
ZORBA_LANGUAGE_EXPECT(header_jsoniq_option jsoniq)
ZORBA_ADD_TEST(bin/zorba_language_header_xquery_j zorbacmd -j -f -q "${_langdir}/header_xquery.q")
ZORBA_LANGUAGE_EXPECT(header_xquery_j xquery)
# ... down to the names a function call resolves: keys() is a JSONiq function
# without a prefix and is not found in a query that declares xquery.
ZORBA_ADD_TEST(bin/zorba_language_header_xquery_names zorbacmd --language jsoniq -f -q "${_langdir}/header_xquery_keys.q")
ZORBA_LANGUAGE_EXPECT(header_xquery_names "XPST0017")
# The error lines are those of the file, whatever the language.
ZORBA_ADD_TEST(bin/zorba_language_lines_jq zorbacmd -f -q "${_langdir}/lines.jq")
ZORBA_LANGUAGE_EXPECT(lines_jq "lines[.]jq>:3,[0-9]+:")
ZORBA_ADD_TEST(bin/zorba_language_lines_xq zorbacmd -f -q "${_langdir}/lines.xq")
ZORBA_LANGUAGE_EXPECT(lines_xq "lines[.]xq>:3,[0-9]+:")
ZORBA_ADD_TEST(bin/zorba_language_lines_syntax_jq zorbacmd -f -q "${_langdir}/lines_syntax.jq")
ZORBA_LANGUAGE_EXPECT(lines_syntax_jq "lines_syntax[.]jq>:3,[0-9]+:")
ZORBA_ADD_TEST(bin/zorba_language_lines_syntax_xq zorbacmd -f -q "${_langdir}/lines_syntax.xq")
ZORBA_LANGUAGE_EXPECT(lines_syntax_xq "lines_syntax[.]xq>:3,[0-9]+:")
ZORBA_ADD_TEST(bin/zorba_language_lines_syntax_option zorbacmd --language jsoniq -f -q "${_langdir}/lines_syntax.xq")
ZORBA_LANGUAGE_EXPECT(lines_syntax_option "lines_syntax[.]xq>:3,[0-9]+:")
# The hints say what to write; none recommends what already applies.
ZORBA_ADD_TEST(bin/zorba_language_hint_xquery zorbacmd -q "let $x := [1] return $x[]")
ZORBA_LANGUAGE_EXPECT(hint_xquery "hint: [^\n]*jsoniq version")
ZORBA_ADD_TEST(bin/zorba_language_hint_jsoniq zorbacmd --language jsoniq -q "[1, ]")
ZORBA_SET_TEST_PROPERTY(bin/zorba_language_hint_jsoniq PASS_REGULAR_EXPRESSION "XPST0003")
ZORBA_SET_TEST_PROPERTY(bin/zorba_language_hint_jsoniq FAIL_REGULAR_EXPRESSION "jsoniq version")
# The help states the order and its limits.
ZORBA_ADD_TEST(bin/zorba_language_help zorbacmd --help)
ZORBA_SET_TEST_PROPERTY(bin/zorba_language_help PASS_REGULAR_EXPRESSION
  "in this order: [(]1[)] a jsoniq version or xquery version declaration at the very start of the query [(]one preceded by a comment is not detected[)]; [(]2[)] for a file, its extension [(][.]jq is JSONiq, [.]xq is XQuery[)]; [(]3[)] this option.*The option does not override an extension: --jsoniq -f q[.]xq is XQuery[.] --parse-only and the parse that --compile-only does first do not apply this rule")
# A value other than xquery and jsoniq is refused.
ZORBA_ADD_TEST(bin/zorba_language_bad_value zorbacmd --language sql -q "1")
ZORBA_LANGUAGE_EXPECT(bad_value "--language argument must be one of")

# Security policy flags: --secure, --allow, --deny, --read-path, --write-path,
# --network-prefix, --private-addresses. Each capability is shown denied and
# permitted. The queries in test/security print a marker on success. A policy
# denial is reported with the error code the failing function documents
# (FODC0002 for fn:doc, FOFL9999 for the file module, XQST0059 for an import)
# and the message "access denied by security policy (requires capability X)";
# a refused external entity is reported as FODC0006 by fn:parse-xml with
# "URI access not allowed"; a module import refused for lack of a
# capability raises ZXQP0067. ZORBA_SECURITY_DENIED matches the code together
# with the phrase, so a permitted access that fails for another reason (e.g. a
# refused connection, also FODC0002) does not pass as a denial. The network
# tests use 127.0.0.1:9, where nothing listens: they need no server, and an
# access the policy permits fails without the denial phrase.
SET (_secdir "${CMAKE_CURRENT_SOURCE_DIR}/test/security")
SET (_secwrite "${CMAKE_CURRENT_BINARY_DIR}/security-write")
FILE (MAKE_DIRECTORY "${_secwrite}")

MACRO (ZORBA_SECURITY_TEST NAME PASS)
  ZORBA_ADD_TEST(bin/zorba_security_${NAME} zorbacmd ${ARGN})
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_${NAME}
    PASS_REGULAR_EXPRESSION "${PASS}")
ENDMACRO (ZORBA_SECURITY_TEST)

# A denial: the error code and the message, in either order.
MACRO (ZORBA_SECURITY_DENIED NAME CODE PHRASE)
  ZORBA_SECURITY_TEST(${NAME} "${CODE}.*${PHRASE}|${PHRASE}.*${CODE}" ${ARGN})
ENDMACRO (ZORBA_SECURITY_DENIED)

# file-read, --read-path
ZORBA_SECURITY_TEST(read_default "cli-doc"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_DENIED(read_deny FODC0002 "access denied by security policy"
  --deny file-read -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_DENIED(read_secure FODC0002 "access denied by security policy"
  --secure -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_secure_allow "cli-doc"
  --secure --allow file-read -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_deny_all_allow "cli-doc"
  --deny all --allow file-read -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_TEST(read_path_in "cli-doc"
  --secure --allow file-read --read-path "${_secdir}/data"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_DENIED(read_path_out FODC0002 "access denied by security policy"
  --read-path "${_secdir}/other"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_DENIED(read_path_dotdot FODC0002 "access denied by security policy"
  --read-path "${_secdir}/data/sub"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}/data/sub/../..")
ZORBA_SECURITY_TEST(read_path_warning "warning: --read-path has no effect"
  --secure --read-path "${_secdir}/data"
  -f -q "${_secdir}/read.xq" -e "dir:=${_secdir}")

# external-entities
ZORBA_SECURITY_TEST(xxe_default "cli-secret"
  -f -q "${_secdir}/xxe.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_DENIED(xxe_deny FODC0006 "URI access not allowed"
  --deny external-entities -f -q "${_secdir}/xxe.xq" -e "dir:=${_secdir}")
ZORBA_SECURITY_DENIED(xxe_secure FODC0006 "URI access not allowed"
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
ZORBA_SECURITY_DENIED(write_deny FOFL9999 "access denied by security policy"
  --deny file-write
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/deny.txt")
ZORBA_SECURITY_DENIED(write_secure ZXQP0067 "requires capability"
  --secure -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/secure.txt")
ZORBA_SECURITY_DENIED(write_no_native ZXQP0067 "requires capability"
  --deny native-modules
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/no_native.txt")
ZORBA_SECURITY_TEST(write_path_in "written"
  --secure --allow file-write,native-modules --write-path "${_secwrite}"
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/in.txt")
ZORBA_SECURITY_DENIED(write_path_out FOFL9999 "access denied by security policy"
  --secure --allow file-write,native-modules --write-path "${_secwrite}/sub"
  -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/out.txt")

# network, --network-prefix, --private-addresses
ZORBA_SECURITY_TEST(network_deny "error:FODC0002:.*access denied by security policy"
  --deny network
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_private "error:FODC0002:.*access denied by security policy"
  --secure --allow network,native-modules
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_private_deny "error:FODC0002:.*access denied by security policy"
  --private-addresses deny
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_private_allow "error:"
  --secure --allow network,native-modules --private-addresses allow
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_network_private_allow
  FAIL_REGULAR_EXPRESSION "access denied by security policy")
ZORBA_SECURITY_TEST(network_prefix_out "error:FODC0002:.*access denied by security policy"
  --network-prefix "http://127.0.0.1:9/allowed/"
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/doc.xml")
ZORBA_SECURITY_TEST(network_prefix_in "error:"
  --secure --allow network,native-modules
  --network-prefix "http://127.0.0.1:9/allowed/"
  -f -q "${_secdir}/network.xq" -e "url:=http://127.0.0.1:9/allowed/doc.xml")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_network_prefix_in
  FAIL_REGULAR_EXPRESSION "access denied by security policy")

# --max-call-depth is --max-udf-call-depth
ZORBA_SECURITY_TEST(max_call_depth "ZXQP0069"
  --max-call-depth 10 -q "let $f := function($f, $n) { if ($n le 0) then 0 else $f($f, $n - 1) } return $f($f, 20)")

# bad capability name
ZORBA_SECURITY_TEST(bad_capability "comma-separated list of"
  --deny file-read,bogus -q "1")

# --directory <dir>: the query runs under the secure policy with file-read
# (restricted to <dir>) and native-modules (restricted to the file module)
# only, $dir:root of the directory module is bound to <dir>, and the result
# is written as JSON, one item per line. The fixture tree is shared with the rbkt tests of the module.
IF (ZORBA_WITH_FILE_ACCESS)
  SET (_dirq "${CMAKE_CURRENT_SOURCE_DIR}/test/directory")
  SET (_dirtree "${CMAKE_SOURCE_DIR}/test/rbkt/Queries/zorba/directory/tree")

  ZORBA_SECURITY_TEST(dir_list
    "{ \"path\" : \"broken/bad.json\", \"format\" : \"json\"[^\n]*\n{ \"path\" : \"broken/bad.xml\""
    --directory "${_dirtree}" -f -q "${_dirq}/list.jq")
  ZORBA_SECURITY_TEST(dir_join
    "{ \"order\" : \"o1\", \"file\" : \"orders/2026/q1.xml\", \"total\" : 100, \"status\" : \"paid\" }\n{ \"order\" : \"o2\""
    --directory "${_dirtree}" -f -q "${_dirq}/join.xq")
  IF (ZORBA_NO_XMLSCHEMA)
    ZORBA_SECURITY_TEST(dir_html_unsupported
      "html-html-false-UNSUPPORTED_FORMAT"
      --directory "${_dirtree}" -f -q "${_dirq}/html-unsupported.xq")
  ELSE (ZORBA_NO_XMLSCHEMA)
    ZORBA_SECURITY_TEST(dir_html
      "html-html-true-HTML report"
      --directory "${_dirtree}" -f -q "${_dirq}/html.xq")
  ENDIF (ZORBA_NO_XMLSCHEMA)
  ZORBA_SECURITY_DENIED(dir_doc_escape FODC0002 "access denied by security policy"
    --directory "${_dirtree}" -f -q "${_dirq}/doc-escape.xq")
  ZORBA_SECURITY_DENIED(dir_symlink_escape FOFL9999 "access denied by security policy"
    --directory "${_dirtree}" -f -q "${_dirq}/symlink-escape.xq")
  ZORBA_SECURITY_TEST(dir_module_escape "INVALID_PATH"
    --directory "${_dirtree}" -f -q "${_dirq}/module-escape.xq")
  ZORBA_SECURITY_DENIED(dir_network FODC0002 "access denied by security policy"
    --directory "${_dirtree}" -f -q "${_dirq}/network.xq")
  ZORBA_SECURITY_DENIED(dir_remote_import XQST0059 "access denied by security policy"
    --directory "${_dirtree}" -f -q "${_dirq}/remote-import.xq")
  ZORBA_SECURITY_DENIED(dir_write FOFL9999 "access denied by security policy"
    --directory "${_dirtree}"
    -f -q "${_secdir}/write.xq" -e "path:=${_secwrite}/directory.txt")
  ZORBA_SECURITY_TEST(dir_env "env:false:false"
    --directory "${_dirtree}" -f -q "${_secdir}/env.xq")
  # Native modules: only the file module (which the directory module uses)
  # is on the policy's list.
  ZORBA_SECURITY_TEST(dir_native_allowed "\"native-1\""
    --directory "${_dirtree}" -f -q "${_dirq}/native-allowed.xq")
  # A wordnet: thesaurus other than the built-in one reads a data file in the
  # library path, outside the directory.
  IF (NOT ZORBA_NO_FULL_TEXT)
    ZORBA_SECURITY_DENIED(dir_wordnet_denied FTST0018 "access denied by security policy"
      --directory "${_dirtree}" -f -q "${_dirq}/wordnet-denied.xq")
  ENDIF (NOT ZORBA_NO_FULL_TEXT)
  ZORBA_SECURITY_DENIED(dir_native_denied ZXQP0067 "http://zorba.io/modules/sleep"
    --directory "${_dirtree}" -f -q "${_dirq}/native-denied.xq")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_dir_native_denied
    FAIL_REGULAR_EXPRESSION "native-loaded")
  # External entities in the documents: the module resolves a relative system
  # identifier against the URI of the file, and the policy decides whether
  # the entity is read. --directory does not grant external-entities, so no
  # entity is read; a policy that grants it with the root as the read path
  # reads the entity next to the document and refuses the one outside the
  # root, in the error record of the file.
  SET (_direnttree "${CMAKE_SOURCE_DIR}/test/rbkt/Queries/zorba/directory/entity-tree")
  ZORBA_SECURITY_TEST(dir_entity_directory
    "docs/note.xml=FODC0006:[^\n]*URI access not allowed"
    --directory "${_direnttree}" -f -q "${_dirq}/entity.xq")
  ZORBA_SECURITY_TEST(dir_entity_outside
    "escape.xml=FODC0006:[^\n]*URI access not allowed"
    --secure --allow external-entities,file-read,native-modules
    --read-path "${_direnttree}"
    -f -q "${_dirq}/entity.xq" -e "root:=${_direnttree}")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_dir_entity_outside
    FAIL_REGULAR_EXPRESSION "TOP-SECRET-ENTITY")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_dir_entity_directory
    FAIL_REGULAR_EXPRESSION "TOP-SECRET-ENTITY|entity next to the document")
  ZORBA_SECURITY_TEST(dir_entity_inside
    "docs/note.xml=entity next to the document"
    --secure --allow external-entities,file-read,native-modules
    --read-path "${_direnttree}"
    -f -q "${_dirq}/entity.xq" -e "root:=${_direnttree}")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_dir_entity_inside
    FAIL_REGULAR_EXPRESSION "TOP-SECRET-ENTITY")
  ZORBA_SECURITY_TEST(dir_widen "--directory cannot be combined with"
    --directory "${_dirtree}" --allow network -q "1")
  ZORBA_SECURITY_TEST(dir_not_directory "--directory: not a directory"
    --directory "${_dirtree}/customers.csv" -q "1")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_security_dir_list FAIL_REGULAR_EXPRESSION
    "link-out|link-dir|hidden|TOP-SECRET|HIDDEN-SECRET|access denied")
  FOREACH (_t dir_join dir_module_escape dir_env)
    ZORBA_SET_TEST_PROPERTY(bin/zorba_security_${_t}
      FAIL_REGULAR_EXPRESSION "TOP-SECRET|HIDDEN-SECRET|access denied")
  ENDFOREACH (_t)

  # zorba-mcp: an MCP server (JSON-RPC over stdio) that runs queries over a
  # directory like --directory does (directory_config.cpp is shared).
  IF (UNIX)
    SET (_mcp_suffix)
    IF (NOT ${ZORBA_STORE_NAME} STREQUAL "simplestore")
      SET (_mcp_suffix "_${ZORBA_STORE_NAME}")
    ENDIF (NOT ${ZORBA_STORE_NAME} STREQUAL "simplestore")

    # The scanner's threads use libxml2 directly.
    FIND_PACKAGE (Threads REQUIRED)
    SET (MCP_LIBS "${LIBXML2_LIBRARIES};Threads::Threads")
    SET (MCP_COMMON_SRCS mcp/json.cpp mcp/mcp_server.cpp mcp/document_cache.cpp
         mcp/scanner.cpp mcp/scan_preview.cpp directory_config.cpp)
    ZORBA_GENERATE_EXE("zorba_mcp" "mcp/zorba_mcp.cpp;${MCP_COMMON_SRCS}"
      "${MCP_LIBS}" "zorba-mcp" "bin")
    ZORBA_GENERATE_EXE("mcp_tests" "mcp/test/mcp_tests.cpp;${MCP_COMMON_SRCS}"
      "${MCP_LIBS}" "" "")
    TARGET_INCLUDE_DIRECTORIES ("mcp_tests${_mcp_suffix}"
      PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mcp")

    ZORBA_GENERATE_EXE("mcp_scan_tests" "mcp/test/scan_tests.cpp;${MCP_COMMON_SRCS}"
      "${MCP_LIBS}" "" "")
    TARGET_INCLUDE_DIRECTORIES ("mcp_scan_tests${_mcp_suffix}"
      PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mcp")

    # Scripted JSON-RPC exchanges against the directory module's fixture
    # tree (see mcp/test/mcp_tests.cpp); "binary" runs zorba-mcp itself.
    SET (_mcpq "${CMAKE_CURRENT_SOURCE_DIR}/test/mcp/queries")
    FOREACH (_t json protocol legacy tools_list list_documents join
             syntax_error escape network native_modules predefined limits binary windows
             optional_params documents_stream hints_jsoniq hints_dash
             hints_nodes hints_names deep_json deep_json_binary
             entity_expansion case_paths html_edge_cases memory_limit language
             language_binary no_write read_functions native_import reflection_random
             cache_hit cache_invalidate cache_evict cache_off cache_policy
             cache_inject cache_size cache_default cache_abort cache_errors
             cache_expansion cache_counters)
      ADD_TEST (NAME bin/mcp/${_t}
        COMMAND "mcp_tests${_mcp_suffix}" ${_t} "${_dirtree}" "${_mcpq}"
          "${_dirq}/join.xq" "$<TARGET_FILE:zorba_mcp${_mcp_suffix}>")
      SET_TESTS_PROPERTIES (bin/mcp/${_t} PROPERTIES
        PASS_REGULAR_EXPRESSION "PASSED: ${_t}"
        FAIL_REGULAR_EXPRESSION "FAILED|TOP-SECRET|HIDDEN-SECRET")
    ENDFOREACH (_t)

    # The help states the order, and what --queries loads.
    ADD_TEST (NAME bin/mcp/language_help
      COMMAND "$<TARGET_FILE:zorba_mcp${_mcp_suffix}>" --help)
    SET_TESTS_PROPERTIES (bin/mcp/language_help PROPERTIES
      PASS_REGULAR_EXPRESSION "A [.]jq\n *file is JSONiq unless it declares xquery version;\n *[.]xq files are not loaded.*in this order: [(]1[)] a jsoniq\n *version or xquery version declaration at the very\n *start of the query [(]one preceded by a comment is\n *not detected[)]; [(]2[)] for a file, its extension [(][.]jq\n *is JSONiq[)]; [(]3[)] this option")

    # A --language value other than xquery and jsoniq is refused.
    ADD_TEST (NAME bin/mcp/language_invalid
      COMMAND "$<TARGET_FILE:zorba_mcp${_mcp_suffix}>"
        --root "${_dirtree}" --language sql)
    SET_TESTS_PROPERTIES (bin/mcp/language_invalid PROPERTIES
      PASS_REGULAR_EXPRESSION "invalid argument: --language")

    # The background scanner (see mcp/test/scan_tests.cpp).
    FOREACH (_t scan_status scan_immediate scan_huge_tag scan_known_dirs
             scan_unreadable scan_deep_tree scan_preview_bound scan_escaped_names
             scan_refill scan_previews scan_mtime scan_refresh
             scan_symlink scan_path scan_malformed scan_deep scan_xxe scan_limit
             scan_memory scan_memory_big_preview scan_memory_walker
             scan_memory_stress scan_memory_option scan_thread_limit
             scan_thread_failure
             scan_no_scan scan_handshake scan_shutdown scan_binary scan_stress
             scan_description)
      ADD_TEST (NAME bin/mcp/${_t}
        COMMAND "mcp_scan_tests${_mcp_suffix}" ${_t}
          "$<TARGET_FILE:zorba_mcp${_mcp_suffix}>")
      SET_TESTS_PROPERTIES (bin/mcp/${_t} PROPERTIES
        PASS_REGULAR_EXPRESSION "PASSED: ${_t}"
        FAIL_REGULAR_EXPRESSION "FAILED")
    ENDFOREACH (_t)
  ENDIF (UNIX)
ENDIF (ZORBA_WITH_FILE_ACCESS)

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

# --profile json writes valid JSON for an iterator that was never opened (the
# body of a cached call): no nan or inf, no counter read from uninitialized
# memory (4294967295 is an unsigned counter that wrapped or was never set).
ZORBA_ADD_TEST(bin/zorba_profile_json zorbacmd --profile json --no-tree-ids --no-serializer
  -f -q "${CMAKE_SOURCE_DIR}/test/rbkt/Queries/zorba/caching/simple-01.xq")
ZORBA_SET_TEST_PROPERTY(bin/zorba_profile_json PASS_REGULAR_EXPRESSION "\"prof-calls\"")
ZORBA_SET_TEST_PROPERTY(bin/zorba_profile_json
  FAIL_REGULAR_EXPRESSION ": -?(nan|inf)|4294967295")

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

# The current directory is on the URI and library paths, except under
# --secure, where a query must not import whatever modules sit in it.
ZORBA_SECURITY_TEST(cwd_module_default "hello, v2"
  -f -q "${_testdir}/versioning/import1.xq")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_cwd_module_default
  WORKING_DIRECTORY "${_uridir}")
# Not found locally, the import falls back to the network, which --secure
# denies: XQST0059 with the policy denial.
ZORBA_SECURITY_DENIED(cwd_module_secure XQST0059 "access denied by security policy"
  --secure -f -q "${_testdir}/versioning/import1.xq")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_cwd_module_secure
  WORKING_DIRECTORY "${_uridir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_cwd_module_secure
  FAIL_REGULAR_EXPRESSION "hello, v2")

# test the --disable-http-resolution flag (--deny remote-import), using the corresponding test query
# from rbkt/zorba/uris/disable-http and a timeout.
# (the test query imports a schema)
IF(NOT ZORBA_NO_XMLSCHEMA)
  ZORBA_ADD_TEST(bin/zorba_disable_http_download zorbacmd
  -f -q "${_testdir}/uris/disable-http.xq" --disable-http-resolution)
  ZORBA_SET_TEST_PROPERTY(bin/zorba_disable_http_download
  PASS_REGULAR_EXPRESSION "XQST0059")
  ZORBA_SET_TEST_PROPERTY(bin/zorba_disable_http_download TIMEOUT 5)
ENDIF(NOT ZORBA_NO_XMLSCHEMA)

SET(PATH_SEP ":")
ZORBA_ADD_TEST(bin/zorba_module_path_1 zorbacmd
  -f -q "${_testdir}/versioning/external1.xq"
  --module-path "${_libdir}${PATH_SEP}${_uridir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_module_path_1
  PASS_REGULAR_EXPRESSION "hello, v2")

# A relative module path is taken against the current directory, also under
# --secure; a path that leaves the current directory does not grant it.
ZORBA_SECURITY_TEST(module_path_relative_secure "hello, v2"
  --secure -f -q "${_testdir}/versioning/import1.xq" --module-path .)
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_module_path_relative_secure
  WORKING_DIRECTORY "${_uridir}")
ZORBA_SECURITY_DENIED(module_path_dotdot_secure XQST0059 "access denied by security policy"
  --secure -f -q "${_testdir}/versioning/import1.xq" --module-path ../elsewhere)
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_module_path_dotdot_secure
  WORKING_DIRECTORY "${_uridir}")
ZORBA_SET_TEST_PROPERTY(bin/zorba_security_module_path_dotdot_secure
  FAIL_REGULAR_EXPRESSION "hello, v2")

# test compilation of the perf script
ZORBA_ADD_TEST(bin/zorba_compilechk4 zorbacmd 
  -q "${CMAKE_CURRENT_SOURCE_DIR}/../test/zperf/src/start.xq" -f --compile-only)
ZORBA_SET_TEST_PROPERTY(bin/zorba_compilechk4
  PASS_REGULAR_EXPRESSION "[]|[XQST0059]")
