(: An xs:decimal compared with an xs:float is cast to xs:float: 0.1 is equal
   to xs:float("0.1"). :)
let $doc := <d><n>0.1</n><n>0.5</n></d>
for $k in (xs:float("0.1"), xs:float("0.5"), xs:float("0.3"))
let $m := for $n in $doc/n where xs:decimal($n) eq $k return $n
return fn:count($m)
