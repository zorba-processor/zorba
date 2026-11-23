(: The effective boolean value of two atomic items raises FORG0006 at the second item, without running the third, at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:node() { $g := $g + 1; <a/> };
try { boolean(for $i in 1 to 3 return local:bump()) } catch err:FORG0006 { $g }
