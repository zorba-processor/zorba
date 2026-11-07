(: A function that constructs a node returns a new node per call. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare function local:mk() { <a/> };
declare variable $n := local:mk#0;
let $s := for $i in 1 to 2 return $n() return $s[1] is $s[2]
