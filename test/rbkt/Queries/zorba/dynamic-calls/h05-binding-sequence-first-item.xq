(: A for that only returns its variable keeps its binding sequence: the whole domain runs before the first item is used. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
((for $x in (for $j in 1 to 3 return $f()) return $x)[1], $g)
