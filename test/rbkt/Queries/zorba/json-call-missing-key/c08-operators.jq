jsoniq version "1.0";
(: A call on a missing key as an operand and as an argument. :)
for $e in ({ "h" : { "st" : 500 } }, { "t" : 1 })
return [ $e.h("st") + 1, boolean($e.h("st")), count($e.h("st")),
         if ($e.h("st") ge 400) then "high" else "low" ]
