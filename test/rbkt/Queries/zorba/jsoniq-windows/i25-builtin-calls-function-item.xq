(: The same, with the call made by fn:for-each. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare %an:sequential function local:bump1($x) { $g := $g + $x; $g };
for sliding window $w in (for $i in 1 to 3 return $i * 10 + local:g())
  start when true
  only end when true
return (for-each(1, local:bump1#1), $w)
