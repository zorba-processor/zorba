(: jn:null() as a key and as a probe value: null equals only null. :)
let $doc := <d><t>1</t><t n=""/></d>
for $k in (jn:null(), 1, 2)
let $m := for $t in $doc/t
          where (if (fn:exists($t/@n)) then jn:null() else xs:integer($t)) = $k
          return $t
return count($m)
