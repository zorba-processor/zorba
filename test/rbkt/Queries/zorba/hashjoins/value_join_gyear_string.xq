(: An xs:gYear eq an xs:string raises XPTY0004, with or without an index. :)
let $doc := <d><y>2001</y><y>2002</y></d>
for $s in ("2001", "2003")
let $m := for $y in $doc/y where xs:gYear($y) eq $s return $y
return count($m)
