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
#ifndef ZORBA_DIAGNOSTICS_QUERY_HINTS_H
#define ZORBA_DIAGNOSTICS_QUERY_HINTS_H

#include <string>
#include <vector>

namespace zorba {

/**
 * The text of ZorbaException::hint(): what to write instead, for errors that
 * come from a form that is valid elsewhere. A hint never changes an error
 * code or message. An empty string means there is no hint for the case.
 */
namespace query_hints {

/**
 * The hint of a syntax error at a location of a query, from the query text.
 *
 * @param query The text of the module being parsed.
 * @param line The 1-based line of the error.
 * @param column The 1-based column (in bytes) of the error.
 * @param xquery_grammar Whether the module is parsed as XQuery, not JSONiq.
 */
std::string syntax_error( std::string const &query, unsigned long line,
                          unsigned long column, bool xquery_grammar );

/**
 * The hint of an unprefixed call in XQuery of a function that exists without
 * a prefix in JSONiq.
 *
 * @param local The local name of the function.
 * @param prefix The prefix of its namespace: "jn" or "math".
 */
std::string jsoniq_function( std::string const &local,
                             std::string const &prefix );

/** The hint of "[[ ]]" without a position. */
std::string empty_array_lookup();

/**
 * The hint of a call with the wrong number of arguments.
 *
 * @param name The function name as written in the error message.
 * @param signatures Its declared forms.
 */
std::string other_arity( std::string const &name,
                         std::vector<std::string> const &signatures );

/**
 * The hint of a variable that is undeclared because "-" is part of its name.
 *
 * @param left The name of the variable in scope, without "$".
 * @param right What follows the "-" in the name; may be empty.
 */
std::string dash_in_variable( std::string const &left,
                              std::string const &right );

/** The hint of an undeclared namespace prefix. */
std::string undeclared_prefix( std::string const &prefix );

/**
 * The hint of a selector that is not an integer, on an array.
 *
 * @param key The selector.
 */
std::string key_on_array( std::string const &key );

/**
 * The hint of a path whose left side is not a node.
 *
 * @param value_type The type of the item, as in error messages.
 */
std::string path_over_item( std::string const &value_type );

/** The hint of a node in the output of the JSON method. */
std::string node_in_json();

} // namespace query_hints
} // namespace zorba

#endif /* ZORBA_DIAGNOSTICS_QUERY_HINTS_H */
/* vim:set et sw=2 ts=2: */
