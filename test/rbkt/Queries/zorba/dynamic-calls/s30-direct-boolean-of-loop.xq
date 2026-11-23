(: fn:boolean stops after a first item that is a node, at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare %an:sequential function local:node() { $g := $g + 1; <a/> };
(boolean(for $i in 1 to 3 return local:node()), $g)
