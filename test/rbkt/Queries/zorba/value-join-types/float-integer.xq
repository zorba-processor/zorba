(: An xs:integer compared with an xs:float is cast to xs:float: 16777217 is
   16777216 as an xs:float. :)
let $doc := <d><f>16777216</f><f>1</f></d>
for $k in (16777217, 16777216, 2)
let $m := for $f in $doc/f where xs:float($f) eq $k return $f
return fn:count($m)
