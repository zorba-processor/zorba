(: fn:filter with a constant input stays in the loop. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare %an:sequential function local:keep($x) { $g := $g + 1; $g mod 2 eq 1 };
for $i in 1 to 4 return filter($i, local:keep#1)
