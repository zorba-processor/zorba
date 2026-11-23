jsoniq version "1.0";
(: Values bound by a let clause, as an alias of a for variable or as the
 : values a FLWOR returns, and values at the end of a lookup path bound by a
 : let or for clause and looked up again, are read only through lookups of
 : literal keys and counts: at -O1 and -O2 only those keys are built, and the
 : results, and the errors with their locations, are those of -O0, where every
 : value is built.  The uses that get values whole follow. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare variable $log :=
  "{\"s\":\"a\",\"r\":\"/x\",\"t\":1,\"h\":{\"st\":500,\"d\":1.5,\"m\":\"GET\",\"c\":{\"n\":1}},\"u\":{\"id\":1}}\n" ||
  "{\"s\":\"b\",\"r\":\"/x\",\"t\":2,\"h\":{\"st\":200,\"d\":3,\"m\":\"PUT\",\"c\":{\"n\":2,\"x\":[1]}},\"g\":[1,{\"st\":2}]}\n" ||
  "{\"s\":\"a\",\"r\":\"/y\",\"t\":4,\"h\":{\"st\":404,\"d\":2,\"m\":\"GET\"}}\n" ||
  "{\"s\":\"a\",\"r\":\"/x\",\"t\":5,\"h\":{\"st\":503,\"d\":7,\"c\":null},\"x\":null}\n" ||
  "{\"s\":\"b\",\"r\":\"/z\",\"t\":7,\"h\":{\"d\":4,\"m\":\"GET\",\"c\":{\"n\":3}}}\n" ||
  "{\"r\":\"/z\",\"t\":8,\"h\":[{\"st\":500}]}\n" ||
  "[{\"s\":\"a\",\"h\":{\"st\":500}}]";
declare variable $errors := (
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"a\",\"t\":2,\"d\":{\"x\":1,\"x\":2}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500,\"st\":501}}",
  "{\"s\":\"a\",\"t\":1,\"d\":tru,\"h\":{\"st\":500}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"b\",\"h\":{\"c\":[1,2",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"b\",\"d\":[1,{\"q\":@}]}" );
declare function local:error($code, $description, $line, $column)
{ { "error" : [ local-name-from-QName($code), $description, $line, $column ] } };

for $s at $i in ($log, $errors)
let $input := if ($i eq 1) then "log" else "error " || ($i - 1)
return (
{ "case" : "an alias of the for variable", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          let $w := $v
          where $w.h.st ge 400
          return [ $w.t, $w.h.d, count($w.u) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "the values a FLWOR returns", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          group by $k := $v.s order by $k
          let $slow := for $e in $v where $e.h.d ge 2 return $e
          return [ $k, count($slow), [ $slow.t ], $slow[1].r ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a let bound to a lookup path", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          let $h := $v.h
          where $h.st ge 400
          return [ $v.t, $h.d, $h.c.n ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a let bound to a lookup path in a group", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          group by $k := $v.s order by $k
          let $h := $v.h
          return [ $k, sum($h.d), [ $h.c.n ], count($h[$$.st ge 400]) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a for clause over a lookup path", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          for $c in $v.h.c
          return [ $c.n, $c.x ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a let over a let bound to a lookup path", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          let $h := $v.h
          let $c := $h.c
          group by $m := $h.m order by $m
          return [ $m, [ $c.n ], count($h.st) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },

(: Get values whole. :)
{ "case" : "the value at a path whole", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          let $h := $v.h
          where $v.t le 4
          return [ $h, $h.st ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "the members of an array at a path", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          let $g := $v.g
          return [ $g[], $g.st ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "an alias given whole", "input" : $input, "result" :
  try { [ for $v in parse-json($s, $options)
          let $w := $v
          where $w.t gt 5
          return serialize($w) ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } }
)
