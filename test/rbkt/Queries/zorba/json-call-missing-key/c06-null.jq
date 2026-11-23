jsoniq version "1.0";
(: A call on a null value. :)
for $e in ({ "h" : { "st" : 500 } }, { "h" : null })
where $e.h("st") ge 400
return $e
