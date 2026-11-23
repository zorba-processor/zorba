(: fn:zero-or-one over a loop of one item returns it, at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:node() { $g := $g + 1; <a/> };
(zero-or-one(for $i in 1 to 1 return local:bump()), $g)
