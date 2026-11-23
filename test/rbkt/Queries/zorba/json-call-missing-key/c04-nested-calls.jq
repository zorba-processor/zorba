jsoniq version "1.0";
(: A missing key at each level of nested calls. :)
for $e in ({ "h" : { "st" : { "c" : 500 } } }, { "t" : 1 }, { "h" : { "x" : 1 } },
           { "h" : { "st" : { "d" : 1 } } })
return [ $e("h")("st")("c"), count($e("h")("st")("c")), exists($e("h")("st")) ]
