(: An untyped value eq an xs:integer raises XPTY0004, with or without an index. :)
declare construction strip;

let $doc := <d><a>1</a></d>
for $k in (1, 2)
let $m := for $a in $doc/a where $a eq $k return $a
return count($m)
