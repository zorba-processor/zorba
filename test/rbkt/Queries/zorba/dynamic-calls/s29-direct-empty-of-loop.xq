(: fn:empty stops after the first item of a loop with a direct sequential call, at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:node() { $g := $g + 1; <a/> };
(empty(for $i in 1 to 3 return local:bump()), $g)
