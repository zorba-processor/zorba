jsoniq version "1.0";
(: A parse whose values are read only through $v.a.b builds only that path at -O1
 : and -O2, and the rest is still checked: the results, and the error codes,
 : descriptions and locations, are those of -O0, where every value is built. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $limit external := 1500;
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare function local:deep($open as string, $close as string, $n as integer) as string
{ string-join(for $i in 1 to $n return $open) || "1" || string-join(for $i in 1 to $n return $close) };
declare function local:error($code, $description, $line, $column)
{ { "error" : [ local-name-from-QName($code), $description, $line, $column ] } };
declare variable $cases := (
  "{\"a\":{\"b\":1,\"c\":[1,2]},\"d\":{\"b\":2}}",
  "{\"a\":[{\"b\":1}],\"b\":2}",
  "{\"a\":{\"b\":{\"x\":[1,{\"y\":2}],\"z\":null}}}",
  "[1,2] {\"a\":{\"b\":3}} [{\"a\":{\"b\":4}}] {\"a\":7} {\"a\":{\"b\":[5]}}",
  "{\"a\":{\"c\":1}}",
  "{\"z\":1,\"a\":{\"b\":\"q\",\"b2\":4}}",
  "{\"a\":{\"b\":1}}\n{\"a\":{\"x\":0,\"b\":2}}\n{\"q\":{\"a\":{\"b\":0}},\"a\":{\"b\":3}}",
  "{\"a\":{\"b\":1},\"d\":{\"x\":1,\"x\":2}}",
  "{\"d\":{\"x\":1,\"x\":2},\"a\":{\"b\":1}}",
  "{\"a\":{\"b\":1,\"b\":2}}",
  "{\"a\":{\"c\":1,\"c\":2,\"b\":1}}",
  "{\"a\":{\"c\":1,\"b\":1,\"c\":2}}",
  "{\"a\":1,\"a\":2}",
  "{\"d\":[{\"x\":1,\"x\":2}],\"a\":{\"b\":1}}",
  "{\"d\":[1,{\"x\":@}],\"a\":{\"b\":1}}",
  "{\"d\":tru,\"a\":{\"b\":1}}",
  "{\"d\":\"abc",
  "{\"d\":\"\\uD800\",\"a\":{\"b\":1}}",
  "{\"d\":01,\"a\":{\"b\":1}}",
  "{\"a\":{\"b\":1}}\n{\"d\":[1,2",
  "{\"a\":{\"b\":1}}\n{\"d\":[1,2]}}",
  "{\"d\":1e999,\"a\":{\"b\":1e999}}",
  "{\"d\":123456789012345678901234567890,\"a\":{\"b\":123456789012345678901234567890}}",
  "{\"d\":" || local:deep("[", "]", $limit - 1) || ",\"a\":{\"b\":1}}",
  "{\"d\":" || local:deep("[", "]", $limit) || ",\"a\":{\"b\":1}}",
  "{\"a\":{\"b\":1,\"c\":" || local:deep("{\"e\":", "}", $limit) || "}}" );
for $s at $i in $cases
let $projected :=
  try { [ for $v in parse-json($s, $options) return $v.a.b ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) }
return { "case" : $i, "result" : $projected }
