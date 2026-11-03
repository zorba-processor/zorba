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
#ifndef ZORBA_MCP_QUERY_PRIMER_H
#define ZORBA_MCP_QUERY_PRIMER_H

namespace zorba {
namespace mcp {

/**
 * The JSONiq primer of the run_query description, read by the agent on every
 * session. It is built from the errors agents make most often, plus the
 * window clause. Every example goes between backticks, and only examples do:
 * the query_primer test runs what is between backticks and asserts its result.
 */
char const query_primer[] =
  "JSONiq primer; each example runs as written. The context item is $$, not "
  "\".\": `(1, 2, 3)[$$ ge 2]`. The \"content\" of a CSV or top-level JSON "
  "array is an array: open it with [] or (), not a key or [n]: "
  "`let $c := [{\"a\": 1}, {\"a\": 2}] "
  "return ($c[](\"a\"), $c()(\"a\"), $c(2)(\"a\"))`. "
  "\"-\" belongs to names, so space it: "
  "`let $a := 3 let $b := 1 return $a - $b`. "
  "Strings take double quotes and \"\\\" escapes in them: "
  "`matches(\"a1\", \"\\\\d\")`. "
  "math: needs no import: `math:pi() gt 3`. "
  "There is no type number: `(1, 2.5, \"a\")[$$ instance of xs:decimal or "
  "$$ instance of xs:double]`. "
  "Windows: `for sliding window $w in (1, 2, 3, 4) start at $s when true "
  "only end at $e when $e - $s eq 1 return sum($w)`.";

} // namespace mcp
} // namespace zorba

#endif /* ZORBA_MCP_QUERY_PRIMER_H */
/* vim:set et sw=2 ts=2: */
