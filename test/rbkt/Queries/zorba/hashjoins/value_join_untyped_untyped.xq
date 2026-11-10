(: Untyped values on both sides are compared as strings, which raises no
   error, so the join uses an index. :)
declare construction strip;

let $doc := <d><a>x</a><a>1</a><b>1</b><b>1.0</b><b>x</b></d>
for $a in $doc/a
let $m := for $b in $doc/b where $b eq $a return $b
return fn:count($m)
