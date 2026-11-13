(: An xs:float compared with an xs:double is cast to xs:double: xs:float("0.1")
   is not equal to xs:double("0.1"), xs:float("0.5") is equal to 0.5. :)
let $doc := <d><n>0.1</n><n>0.5</n></d>
for $k in (xs:float("0.1"), xs:float("0.5"))
let $m := for $n in $doc/n where xs:double($n) eq $k return $n
return fn:count($m)
