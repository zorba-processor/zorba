jsoniq version "1.0";
(: castable as filters values that cannot be cast, null among them (JSONiq
   extension 6.14: null casts only to xs:string and xs:untypedAtomic). :)
let $a := [ {"v":"1.5"}, {"v":2}, {"v":"n/a"}, {"v":null} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return sum(for $x in $w.v where $x castable as xs:decimal return xs:decimal($x))
]
