(: An untyped value is compared with an xs:anyURI as a string, which raises
   no error, so the join uses an index with normalized keys. :)
declare construction strip;

let $doc := <d><u>a</u><u>b</u><u>a</u></d>
for $k in (xs:anyURI("a"), xs:anyURI("c"))
let $m := for $u in $doc/u where $u eq $k return $u
return fn:count($m)
