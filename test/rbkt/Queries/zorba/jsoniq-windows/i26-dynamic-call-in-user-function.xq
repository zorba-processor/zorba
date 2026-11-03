(: The same, with the call inside a function that is not declared sequential. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare function local:call($h) { $h() };
for sliding window $w in (for $i in 1 to 3 return $i * 10 + local:g())
  start when true
  only end when true
return (local:call($f), $w)
