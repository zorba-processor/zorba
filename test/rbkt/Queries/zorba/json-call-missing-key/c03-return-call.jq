jsoniq version "1.0";
for $e in ({ "h" : { "st" : 500 } }, { "t" : 1 }, { "h" : { "x" : 1 } })
return [ $e.h("st") ]
