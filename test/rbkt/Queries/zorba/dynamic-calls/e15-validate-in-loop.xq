(: fn validate gives a new node per call. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare variable $d := <a/>;
declare function local:v() { validate lax { $d } };
declare variable $p := local:v#0;
let $s := for $i in 1 to 2 return $p() return $s[1] is $s[2]
