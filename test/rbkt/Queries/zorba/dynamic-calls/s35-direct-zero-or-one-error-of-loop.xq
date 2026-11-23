(: fn:zero-or-one raises FORG0003 at the second item of a loop, without running the third, at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:node() { $g := $g + 1; <a/> };
try { zero-or-one(for $i in 1 to 3 return local:bump()) } catch err:FORG0003 { $g }
