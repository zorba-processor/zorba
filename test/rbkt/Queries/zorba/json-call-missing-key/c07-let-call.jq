jsoniq version "1.0";
for $e in ({ "h" : { "st" : 500 } }, { "t" : 1 })
let $h := $e.h
where $h("st") ge 400
return $e
