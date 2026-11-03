(: fn:exists runs the call although the type gives the result. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:bump1($x) { $g := $g + 1; $g };
declare %an:nondeterministic function local:g() { $g };
declare variable $f := local:bump#0;
declare variable $f1 := local:bump1#1;
declare %an:sequential function local:n() as xs:integer { $g := $g + 1; $g };
declare variable $t as function() as xs:integer := local:n#0;
(exists($t()), $g)
