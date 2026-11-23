jsoniq version "1.0";
(: A call on a missing key gives the empty sequence, so the where clause is false. :)
for $e in ({ "h" : { "st" : 500 } }, { "t" : 1 })
where $e.h("st") ge 400
return $e
