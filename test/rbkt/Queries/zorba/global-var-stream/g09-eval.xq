(: eval can read $g by name: $g is materialized. :)
import module namespace refl = "http://zorba.io/modules/reflection";

declare variable $g := (1, 2);

for $x in $g
return refl:eval("fn:count($g)")
