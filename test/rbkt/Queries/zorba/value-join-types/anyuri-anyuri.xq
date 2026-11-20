(: xs:anyURI eq xs:anyURI compares strings and raises no error, so the join
   uses an index. :)
let $doc := <d><u>a</u><u>b</u><u>a</u></d>
for $k in (xs:anyURI("a"), xs:anyURI("c"))
let $m := for $u in $doc/u where xs:anyURI($u) eq $k return $u
return fn:count($m)
