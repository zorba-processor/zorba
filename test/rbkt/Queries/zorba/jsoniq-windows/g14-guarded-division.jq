jsoniq version "1.0";
(: XQuery 3.0 2.3.4: the branch that is not selected raises no error. :)
let $a := [ {"v":0}, {"v":2}, {"v":"x"} ]
return [
  for tumbling window $w in $a[]
    start $s when if ($s.v instance of xs:integer and $s.v ne 0) then 1 div $s.v gt 0 else false
  return [ $w.v ]
]
