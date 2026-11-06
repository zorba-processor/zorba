(: The same, two calls away and recursive. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare function local:a($h, $n) { if ($n gt 0) then local:b($h, $n - 1) else $h() };
declare function local:b($h, $n) { local:a($h, $n) };
for $i in 1 to 3 return local:a($f, 1)
