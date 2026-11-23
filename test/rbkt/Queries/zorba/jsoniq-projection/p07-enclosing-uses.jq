jsoniq version "1.0";
(: A parse returned by a user function that calls another one, the values of a
 : window clause over a parse, values taken by position, values that a FLWOR
 : gives to an enclosing lookup or for clause, and a lookup in a parse, are read
 : only through lookups of literal keys and counts: at -O1 and -O2 only those
 : keys are built, and the results, and the errors with their locations, are
 : those of -O0, where every value is built.  The uses that get values whole
 : follow. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare namespace an = "http://zorba.io/annotations";
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare variable $log :=
  "{\"s\":\"a\",\"r\":\"/x\",\"t\":1,\"h\":{\"st\":500,\"d\":1.5,\"m\":\"GET\"},\"u\":{\"id\":1}}\n" ||
  "{\"s\":\"b\",\"r\":\"/x\",\"t\":2,\"h\":{\"st\":200,\"d\":3,\"m\":\"PUT\"},\"g\":[1,{\"st\":2}]}\n" ||
  "{\"s\":\"a\",\"r\":\"/y\",\"t\":4,\"h\":{\"st\":404,\"d\":2,\"m\":\"GET\"}}\n" ||
  "{\"s\":\"a\",\"r\":\"/x\",\"t\":5,\"h\":{\"st\":503,\"d\":7,\"m\":\"GET\"},\"x\":null}\n" ||
  "{\"s\":\"b\",\"r\":\"/z\",\"t\":7,\"h\":{\"d\":4,\"m\":\"GET\"}}\n" ||
  "{\"r\":\"/z\",\"t\":8,\"h\":[{\"st\":500}]}\n" ||
  "[{\"s\":\"a\",\"h\":{\"st\":500}}]";
declare variable $errors := (
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"a\",\"t\":2,\"d\":{\"x\":1,\"x\":2}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500,\"st\":501}}",
  "{\"s\":\"a\",\"t\":1,\"d\":tru,\"h\":{\"st\":500}}",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"b\",\"d\":[1,2",
  "{\"s\":\"a\",\"t\":1,\"h\":{\"st\":500}}\n{\"s\":\"b\",\"d\":[1,{\"q\":@}]}" );
declare function local:error($code, $description, $line, $column)
{ { "error" : [ local-name-from-QName($code), $description, $line, $column ] } };
declare function local:text($s as string) as string { $s };
declare %an:nondeterministic function local:events($s as string)
{ parse-json(local:text($s), $options) };
declare function local:events-from($s as string, $n as integer)
{ if ($n le 0) then parse-json($s, $options) else local:events-from($s, $n - 1) };

for $s at $i in ($log, $errors)
let $input := if ($i eq 1) then "log" else "error " || ($i - 1)
return (
{ "case" : "a parse returned by a function that calls another", "input" : $input, "result" :
  try { [ for $e in local:events($s) group by $k := $e.s order by $k
          return [ $k, count($e), sum($e.h.d), [ $e[$$.h.st ge 500].r ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "values by position", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options) group by $k := $e.s order by $k
          return [ $k, $e[1].r, $e[2].h.st, $e[count($e)].t ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a tumbling window", "input" : $input, "result" :
  try { [ for tumbling window $w in parse-json($s, $options)
            start $first when true()
            end next $n when floor($n.t div 3) gt floor($first.t div 3)
          return [ $first.t, $w[1].s, count($w),
                   [ for $e in $w group by $k := $e.s order by $k return [ $k, count($e) ] ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a sliding window", "input" : $input, "result" :
  try { [ for sliding window $w in parse-json($s, $options)
            start at $a when true()
            only end $last at $b when $b - $a eq 2
          return [ $a, $last.r, sum($w.h.d), count($w[$$.h.st ge 500]) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a window over a group's values", "input" : $input, "result" :
  try { [ for $e in parse-json($s, $options) group by $k := $e.s order by $k
          return [ $k, for tumbling window $w in $e start when true() end at $j when $j mod 2 eq 0
                       return sum($w.t) ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "values given to a lookup", "input" : $input, "result" :
  try { [ (for $v in parse-json($s, $options) where $v.h.st ge 400 return $v).h.d ],
        [ parse-json($s, $options)[$$.s eq "a"].r ],
        [ local:events($s)[$$.h.m eq "GET"].h.st ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "values given to an enclosing for clause", "input" : $input, "result" :
  try { [ for $x in (for $e in local:events($s) group by $r := $e.r
                     where every $m in $e.h.m satisfies $m eq "GET"
                     return $e)
          group by $k := $x.s order by $k
          return [ $k, count($x), [ $x.t ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a lookup in a parse", "input" : $input, "result" :
  try { [ max(local:events($s).t), [ parse-json($s, $options).h.st ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },

(: Get the values whole. :)
{ "case" : "a window whole", "input" : $input, "result" :
  try { [ for tumbling window $w in parse-json($s, $options)
            start $first when true() end at $j when $j mod 3 eq 0
          return [ $first, count($w), [ $w ] ] ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "values given whole to the enclosing expression", "input" : $input, "result" :
  try { [ for $x in (for $v in parse-json($s, $options) return $v) where $x.t gt 4 return $x ],
        serialize(for $v in parse-json($s, $options) where $v.s eq "b" return $v) }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } },
{ "case" : "a parse returned by a recursive function", "input" : $input, "result" :
  try { [ for $e in local:events-from($s, 2) return $e.h.st ] }
  catch * { local:error($err:code, $err:description, $err:line-number, $err:column-number) } }
)
