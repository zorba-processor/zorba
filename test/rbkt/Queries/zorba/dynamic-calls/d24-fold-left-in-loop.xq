(: fn:fold-left with a constant input stays in the loop. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare %an:sequential function local:acc($a, $b) { $g := $g + 1; $g };
declare variable $h as function(item()*, item()) as item()* := local:acc#2;
for $i in 1 to 2 return fold-left(1, 0, $h)
