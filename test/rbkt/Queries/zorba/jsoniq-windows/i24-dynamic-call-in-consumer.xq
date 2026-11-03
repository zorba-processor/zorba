(: The same, with the call in the FLWOR that consumes the windows one by one.
   The call depends on the tuple, so that the optimizer keeps it in the loop. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#1;
for $x in (
  for sliding window $w in (for $i in 1 to 3 return $i * 10 + local:g())
    start when true
    only end when true
  return $w)
return ($f($x), $x)
