(: Values whose keys repeat, shift, are missing, are added or come in another order
   than in the value before keep exactly their own keys, in their own order. :)
declare function local:show($v as item()) as xs:string
{
  if ($v instance of object())
  then "{" || string-join(for $k in jn:keys($v) return $k || ":" || local:show($v($k)), ",") || "}"
  else string($v)
};

let $log := '{"a":1,"b":2,"c":3}
{"a":4,"c":5}
{"x":0,"a":6,"b":7,"c":8}
{"c":9,"b":10,"a":11}
{"a":{"b":1,"c":2},"b":3}
{"a":{"c":4},"b":5,"d":6}
{"e":7}
{"a":8,"b":9,"c":10}'
return string-join(
  for $v in jn:parse-json($log, { "jsoniq-multiple-top-level-items" : true() })
  return local:show($v),
  " | ")
