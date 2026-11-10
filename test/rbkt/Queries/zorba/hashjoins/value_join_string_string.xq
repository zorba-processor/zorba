(: xs:string eq xs:string raises no error, so the join uses an index. :)
let $doc := <d><b>x</b><b>z</b><b>x</b></d>
for $k in ("x", "y", "z")
let $m := for $b in $doc/b where fn:string($b) eq $k return $b
return fn:count($m)
