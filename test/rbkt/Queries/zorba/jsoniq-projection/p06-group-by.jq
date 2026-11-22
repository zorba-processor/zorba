jsoniq version "1.0";
(: After group by, the for variable over a parse is bound to the values of each
 : group. Counting them, predicates over them that look up literal keys in $$,
 : and lookups of literal keys in them, also as $v("k"), build only the keys they
 : name at -O1 and -O2: the results, and the errors with their locations, are
 : those of -O0, where every value is built. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare variable $log :=
  "{\"s\":\"a\",\"r\":\"/x\",\"h\":{\"st\":500,\"d\":1.5,\"m\":\"GET\"},\"u\":{\"id\":1}}\n" ||
  "{\"s\":\"b\",\"r\":\"/x\",\"h\":{\"st\":200,\"d\":3,\"m\":\"PUT\"},\"t\":[1,{\"st\":2}]}\n" ||
  "{\"s\":\"a\",\"r\":\"/y\",\"h\":{\"st\":404,\"d\":2,\"m\":\"GET\"}}\n" ||
  "{\"s\":\"a\",\"r\":\"/x\",\"h\":{\"st\":503,\"d\":7,\"m\":\"POST\"},\"x\":null}\n" ||
  "{\"s\":\"b\",\"r\":\"/z\",\"h\":{\"d\":4,\"m\":\"GET\"}}\n" ||
  "{\"r\":\"/z\",\"h\":[{\"st\":500}]}\n" ||
  "[{\"s\":\"a\",\"h\":{\"st\":500}}]";
declare variable $objects :=
  "{\"s\":\"a\",\"h\":{\"st\":500,\"m\":\"GET\"},\"u\":{\"id\":1,\"n\":\"p\"}}\n" ||
  "{\"s\":\"b\",\"h\":{\"st\":200,\"m\":\"PUT\"},\"1\":\"one\"}\n" ||
  "{\"s\":\"a\",\"h\":{\"st\":404},\"u\":{\"id\":2}}";
declare variable $errors := (
  "{\"s\":\"a\",\"h\":{\"st\":500}}\n{\"s\":\"a\",\"d\":{\"x\":1,\"x\":2}}",
  "{\"s\":\"a\",\"h\":{\"st\":500,\"st\":501}}",
  "{\"s\":\"a\",\"d\":tru,\"h\":{\"st\":500}}",
  "{\"s\":\"a\",\"h\":{\"st\":500}}\n{\"s\":\"b\",\"d\":[1,2",
  "{\"s\":\"a\",\"h\":{\"st\":500}}\n{\"s\":\"b\",\"d\":[1,{\"q\":@}]}" );
declare function local:error($code, $description, $line, $column)
{ { "error" : [ local-name-from-QName($code), $description, $line, $column ] } };

(: Read only what they name. :)
{ "case" : "count, empty, exists", "result" :
  try { [ for $e in parse-json($log, $options) group by $s := $e.s order by $s
          return [ $s, count($e), empty($e), exists($e) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "predicates", "result" :
  try { [ for $e in parse-json($log, $options) group by $s := $e.s order by $s
          return [ $s, count($e[$$.h.st ge 500]), exists($e[$$.h.m eq "GET"]),
                   empty($e[$$.u.id eq 1]), count($e[$$.h.st eq max($e.h.st)]) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "aggregates", "result" :
  try { [ for $e in parse-json($log, $options) group by $s := $e.s order by $s
          return [ $s, sum($e.h.d), max($e.h.d), min($e.h.d), avg($e.h.d),
                   [ distinct-values($e.h.m) ], string-join($e.r, ","),
                   [ $e.t ], [ $e.h.st ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "lookups in a predicate's values", "result" :
  try { [ for $e in parse-json($log, $options) group by $s := $e.s order by $s
          return [ $s, [ $e[$$.h.st ge 400].h.d ], sum($e[$$.h.m eq "GET"].h.d) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "latency", "result" :
  try { [ for $e in parse-json($log, $options)
          group by $service := $e.s, $route := $e.r
          let $ms := $e.h.d
          order by avg($ms) descending, $service, $route
          return { "service" : $service, "route" : $route, "requests" : count($e),
                   "avg_ms" : round(avg($ms), 3), "max_ms" : max($ms),
                   "server_errors" : count($e[$$.h.st ge 500]) } ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "two group by", "result" :
  try { [ for $e in parse-json($log, $options) group by $s := $e.s
          let $n := count($e) group by $n order by $n
          return [ $n, count($e), [ $e.r ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "call syntax", "result" :
  try { [ for $e in parse-json($objects, $options) group by $s := $e("s") order by $s
          return [ $s, [ $e("h")("st") ], [ $e("u")("n") ], [ $e("u").id ] ] ],
        [ for $v in parse-json($objects, $options) return ($v("h")("m"), $v("1x")) ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
for $s at $i in $errors
return { "case" : "error " || $i, "result" :
  try { [ for $e in parse-json($s, $options) group by $k := $e.s
          return [ $k, count($e), count($e[$$.h.st ge 500]) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },

(: Get the values whole. :)
{ "case" : "a predicate's values", "result" :
  [ for $e in parse-json($log, $options) group by $s := $e.s order by $s
    return [ $s, $e[$$.h.st ge 500] ] ] },
{ "case" : "a predicate that reads $$ whole", "result" :
  [ for $e in parse-json($objects, $options) group by $s := $e.s order by $s
    return [ $s, count($e[count(keys($$)) gt 2]), count($e[serialize($$) eq "x"]) ] ] },
{ "case" : "a value by position", "result" :
  [ for $e in parse-json($log, $options) group by $s := $e.s order by $s
    return [ $s, $e[1] ] ] },
{ "case" : "a for clause over the group", "result" :
  [ for $e in parse-json($objects, $options) group by $s := $e.s order by $s
    return [ $s, for $x in $e return $x ] ] },
{ "case" : "keys that select members", "result" :
  [ for $v in parse-json("[10,20] [30] {\"1\":40}", $options)
    return ($v("1"), $v(" +2 ")) ] },
{ "case" : "the group whole", "result" :
  [ for $e in parse-json($objects, $options) group by $s := $e.s order by $s
    return [ $s, count($e), serialize([ $e ]) ] ] }
