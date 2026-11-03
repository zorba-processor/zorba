(: The same, with the call in the end condition. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
for sliding window $w in (for $i in 1 to 3 return $i * 10 + local:g())
  start when true
  only end when exists($f())
return $w
