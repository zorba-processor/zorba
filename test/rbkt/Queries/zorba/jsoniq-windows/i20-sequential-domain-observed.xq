(: A sequential domain is read completely before the first condition: $g is 3 for all of them.
   Read as it goes, $g is 1 at the first item and the windows would all end at the last one. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:step($x) { $g := $x; $x };
for sliding window $w in (local:step(1), local:step(2), local:step(3))
  start when true
  only end when $g eq 3
return string-join(for $x in $w return string($x), "-")
