jsoniq version "1.0";
(: Without the option and with it: the same members, or the same error code,
 : description and location. An error located in the JSON has the same line on both
 : sides; one located in the query is at the call, line 80 without the option and line
 : 81 with it, on the same column. The calls are in the main module, not in a function,
 : so that bin/zorba_plan_parity_O* can check that the plan streams at every
 : optimization level. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: External, so that the compiler does not fold the deep inputs into the plan. :)
declare variable $limit external := 1500;
declare function local:deep($open as string, $close as string, $n as integer) as string
{ string-join(for $i in 1 to $n return $open) || "1" || string-join(for $i in 1 to $n return $close) };
declare function local:error($code, $description, $column, $line)
{ { "e" : [ local-name-from-QName($code), $description, $column ], "l" : $line } };
declare variable $cases := (
  "[1,2,",
  "[1 2]",
  "[1,]",
  "[,1]",
  "{\"a\":1,}",
  "{\"a\"}",
  "{a:1}",
  "[tru]",
  "[nul]",
  "[01]",
  "[1.]",
  "[1e]",
  "[-]",
  "[\"\\q\"]",
  "[\"\\uD800\"]",
  "[\"\\u12\"]",
  "[\"abc",
  "[1] 2",
  "5",
  "\"s\"",
  "]",
  "[1]]",
  "}",
  "{",
  "[",
  "",
  " ",
  "{\"a\":1,\"a\":2}",
  "[{\"a\":1,\"a\":2},@]",
  "[{\"a\":1,\"a\":2,@]",
  "[{\"a\":{\"b\":1,\"b\":2},\"a\":3}]",
  "[1,{\"a\":[{\"b\":1,\"b\":2}]}]",
  "{\"a\":1}{\"a\":2,\"a\":3}",
  "[1]{\"a\":1,\"a\":2}",
  "[1][2,@",
  "[{\"k0\":0,\"k1\":1,\"k2\":2,\"k3\":3,\"k4\":4,\"k5\":5,\"k6\":6,\"k7\":7,\"k8\":8,\"k9\":9,\"k10\":10,\"k11\":11,\"k12\":12,\"k13\":13,\"k14\":14,\"k15\":15,\"k16\":16,\"k17\":17,\"k18\":18,\"k19\":19,\"k3\":0}]",
  "[{\"k0\":0,\"k1\":1,\"k2\":2,\"k3\":3,\"k4\":4,\"k5\":5,\"k6\":6,\"k7\":7,\"k8\":8,\"k9\":9,\"k10\":10,\"k11\":11,\"k12\":12,\"k13\":13,\"k14\":14,\"k15\":15,\"k16\":16,\"k17\":17,\"k18\":18,\"k19\":19}]",
  "[{\"k0\":0,\"k1\":1,\"k2\":2,\"k3\":3,\"k4\":4,\"k5\":5,\"k6\":6,\"k7\":7,\"k8\":8,\"k9\":9,\"k10\":10,\"k11\":11,\"k12\":12,\"k13\":13,\"k14\":14,\"k15\":15,\"k16\":16,\"k17\":17,\"k18\":18,\"k19\":19,\"k3\":0,@]",
  codepoints-to-string(65279) || "[1,2]",
  codepoints-to-string(65279) || "[1,2",
  codepoints-to-string((65279, 65279)) || "[1]",
  codepoints-to-string((65279, 65279)),
  "[1e999]",
  "[-1e999]",
  "[1.5e-999]",
  "[1E+2]",
  "[123456789012345678901234567890]",
  "[0.1234567890123456789012345678901234567890]",
  "[\"a\nb\"]",
  "[1,\n2,\n3 4]",
  "[true,false,null]",
  "[[1,[2,[3]]],{\"a\":[]},\"\\u00e9\"]",
  "[] []",
  "[[]]",
  "{\"a\":[1,2]}",
  "[1] {\"b\":2} [3]",
  local:deep("[", "]", $limit - 1), local:deep("[", "]", $limit), local:deep("[", "]", $limit + 1),
  "[" || local:deep("{\"a\":", "}", $limit - 1) || "]", "[" || local:deep("{\"a\":", "}", $limit) || "]",
  local:deep("{\"a\":", "}", $limit), local:deep("{\"a\":", "}", $limit + 1) );
declare function local:same($a, $b)
{ if ($a instance of array) then deep-equal($a, $b)
  else $b instance of object and deep-equal($a.e, $b.e) and ($a.l eq $b.l or ($a.l eq 80 and $b.l eq 81)) };
let $results :=
  for $s in $cases
  let $without := try { [ parse-json($s)[] ] } catch * { local:error($err:code, $err:description, $err:column-number, $err:line-number) }
  let $with :=    try { [ parse-json($s, { "zorba-streaming-members" : true })[] ] } catch * { local:error($err:code, $err:description, $err:column-number, $err:line-number) }
  return { "same" : local:same($without, $with), "error" : $without instance of object }
let $diff := for $r at $i in $results where not($r.same) return $i
return "cases: " || count($cases) || ", more than 30 errors: " || string(count($results[$$.error]) gt 30) || ", different: " || string-join($diff ! string($$), " ")
