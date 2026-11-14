(: An xs:anyURI is compared with an untyped value as a string, which raises
   no error, so the join uses an index with normalized keys. :)
declare construction strip;

let $doc := <d><u>a</u><u>b</u><u>a</u></d>
for $k in (<k>a</k>, <k>c</k>)
let $m := for $u in $doc/u where xs:anyURI($u) eq $k return $u
return fn:count($m)
