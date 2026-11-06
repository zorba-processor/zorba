(: A function with a side effect that is not sequential is not moved out either. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare %an:sequential function local:t() { $g := $g + 1; trace($g, "t") };
declare variable $h := local:t#0;
for $i in 1 to 2 return $h()
