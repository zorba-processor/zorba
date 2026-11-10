(: xs:integer eq xs:integer raises no error, so the join uses an index. :)
let $doc := <d><b>1</b><b>2</b><b>1</b></d>
for $k in (1, 2, 3)
let $m := for $b in $doc/b where xs:integer($b) eq $k return $b
return fn:count($m)
