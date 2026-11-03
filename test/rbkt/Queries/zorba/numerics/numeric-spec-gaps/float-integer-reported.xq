(: F&O 3.0 op:numeric-equal and B.1 promotion: an xs:integer compared with an
   xs:float is promoted by casting to xs:float, so 16777217 eq xs:float(16777216).
   F&O 3.0 fn:distinct-values: "removing from $arg all but one of a set of values
   that are equal to one another", and XQuery 3.0 3.10.7: grouping keys are
   equivalent if deep-equal, so both must merge the two values. :)
let $i := 16777217
let $f := xs:float(16777216)
return ($i eq $f, count(distinct-values(($i, $f))), count(for $x in ($i, $f) group by $k := $x return $k))
