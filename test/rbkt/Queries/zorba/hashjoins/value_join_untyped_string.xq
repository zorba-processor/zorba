(: An untyped value is compared with an xs:string as a string, which raises
   no error, so the join uses an index with normalized keys. :)
declare construction strip;

let $doc := <d><b>1</b><b>1.0</b><b>x</b></d>
for $k in ("1", "x", "y")
let $m := for $b in $doc/b where $b eq $k return $b
return fn:count($m)
