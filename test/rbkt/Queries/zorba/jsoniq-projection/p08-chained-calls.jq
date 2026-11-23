jsoniq version "1.0";
(: Chained calls of literal keys, $e("h")("st"), are read as the dot form
 : $e.h.st is: at -O1 and -O2 only the keys looked up are built, and the
 : results, and the errors with their locations, are those of -O0, where every
 : value is built.  A call of a key on an array is an error at every level. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare variable $log :=
  "{\"s\":\"a\",\"r\":\"/x\",\"t\":1,\"h\":{\"st\":500,\"d\":1.5,\"m\":\"GET\",\"c\":{\"n\":1}},\"u\":{\"id\":1}}\n" ||
  "{\"s\":\"b\",\"r\":\"/x\",\"t\":2,\"h\":{\"st\":200,\"d\":3,\"m\":\"PUT\",\"c\":{\"n\":2,\"x\":[1]}}}\n" ||
  "{\"s\":\"a\",\"r\":\"/y\",\"t\":4,\"h\":{\"st\":404,\"d\":2,\"m\":\"GET\"}}\n" ||
  "{\"s\":\"a\",\"r\":\"/x\",\"t\":5,\"h\":{\"st\":503,\"d\":7,\"c\":{}},\"x\":null}\n" ||
  "{\"s\":\"b\",\"r\":\"/z\",\"t\":7,\"h\":{\"d\":4,\"m\":\"GET\",\"c\":{\"n\":3}}}\n" ||
  "{\"r\":\"/z\",\"t\":8}";
declare variable $inputs := (
  $log,
  $log || "\n{\"s\":\"c\",\"t\":9,\"h\":[{\"st\":500}]}",
  $log || "\n{\"s\":\"c\",\"t\":9,\"h\":{\"st\":500,\"c\":null}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"a\",\"t\":2,\"d\":{\"x\":1,\"x\":2}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500,\"st\":501}}",
  "{\"s\":\"a\",\"t\":1,\"d\":tru,\"h\":{\"st\":500}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"b\",\"h\":{\"c\":[1,2",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"b\",\"d\":[1,{\"q\":@}]}" );
declare function local:error($code, $description, $line, $column)
{ { "error" : [ local-name-from-QName($code), $description, $line, $column ] } };

for $s at $i in $inputs
let $input := if ($i le 3) then ("log", "an array", "a null")[$i] else "error " || ($i - 3)
return (
{ "case" : "chained calls", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options)
          return [ $e("t"), $e("h")("st"), $e("h")("c")("n") ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "calls and lookups mixed", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options)[$$.t ne 8]
          where $e.h("st") ge 400
          return [ $e("h").d, $e.h("c").n ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "chained calls on the values of a group", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options) group by $k := $e("s") order by $k
          return [ $k, count($e), sum($e("h")("d")), [ $e("h")("c")("n") ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a chained call given to an enclosing lookup", "input" : $input, "result" :
  try { [ parse-json($s, $options)("h")("c").n ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },

(: Get values whole. :)
{ "case" : "the value of a chained call whole", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options) return $e("h")("c") ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a chained call with a member position", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options) return $e("h")("c")("x")(1) ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } }
)
