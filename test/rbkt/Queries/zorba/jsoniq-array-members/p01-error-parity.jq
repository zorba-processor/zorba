jsoniq version "1.0";
(: Without the option and with it: the same members, or the same error code,
 : description and location. The two calls are the same number of lines apart in every
 : case, on the same column. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare function local:deep($open as string, $close as string, $n as integer) as string
{ string-join(for $i in 1 to $n return $open) || "1" || string-join(for $i in 1 to $n return $close) };
declare function local:without($s as string)
{ try { [ parse-json($s)[] ] }
  catch * { { "e" : [ local-name-from-QName($err:code), $err:description, $err:column-number ], "l" : $err:line-number } } };
declare function local:with($s as string)
{ try { [ parse-json($s, { "zorba-streaming-members" : true })[] ] }
  catch * { { "e" : [ local-name-from-QName($err:code), $err:description, $err:column-number ], "l" : $err:line-number } } };
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
  "[1e999]",
  "[-1e999]",
  "[1.5e-999]",
  "[1E+2]",
  "[123456789012345678901234567890]",
  "[0.1234567890123456789012345678901234567890]",
  "[\"a\nb\"]",
  "[true,false,null]",
  "[[1,[2,[3]]],{\"a\":[]},\"\\u00e9\"]",
  "[] []",
  "[[]]",
  "{\"a\":[1,2]}",
  "[1] {\"b\":2} [3]",
  local:deep("[", "]", 1499), local:deep("[", "]", 1500), local:deep("[", "]", 1501),
  "[" || local:deep("{\"a\":", "}", 1499) || "]", "[" || local:deep("{\"a\":", "}", 1500) || "]",
  local:deep("{\"a\":", "}", 1500), local:deep("{\"a\":", "}", 1501) );
declare function local:same($a, $b)
{ if ($a instance of array) then deep-equal($a, $b)
  else $b instance of object and deep-equal($a.e, $b.e) and ($a.l eq $b.l or $a.l + 3 eq $b.l) };
let $diff := for $s at $i in $cases
             where not(local:same(local:without($s), local:with($s)))
             return $i
return "cases: " || count($cases) || ", more than 30 errors: " || string(count($cases[local:without($$) instance of object]) gt 30) || ", different: " || string-join($diff ! string($$), " ")
