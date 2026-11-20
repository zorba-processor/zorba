(: A count compared with a number still counts every item of a loop with assignments: all of them run at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
(count(for $i in 1 to 5 return { $g := $g + 1; $g }) = 3, $g)
