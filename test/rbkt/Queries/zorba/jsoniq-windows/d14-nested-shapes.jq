jsoniq version "1.0";
(: JSONiq 5.5.1, 5.5.2: lookups on the wrong kind of item are empty, whatever
   the shape of each record. :)
let $a := [ {"a":{"b":1}}, {"a":[{"b":2}]}, {"a":"x"}, {"a":{"b":{"c":3}}}, {} ]
return [
  [ for tumbling window $w in $a[] start at $i when true end at $j when $j - $i eq 1 return [ $w.a.b ] ],
  [ for tumbling window $w in $a[] start at $i when true end at $j when $j - $i eq 1 return [ $w.a[].b ] ]
]
