(: The same, with the call in the start condition.
   The calls depend on the tuple and on a function item, so that the optimizer keeps them in the loops. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g($i) { $g };
declare variable $read := local:g#1;
declare variable $f := local:bump#1;
for sliding window $w in (for $i in 1 to 3 return $i * 10 + $read($i))
  start $s when exists($f($s))
  only end when true
return $w
