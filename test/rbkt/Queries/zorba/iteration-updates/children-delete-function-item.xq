(: A function item called in the return clause, with the bound child as argument, deletes the following children of the node whose children the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/><b k="2"/><b k="3"/><b k="4"/></a>;
declare %an:sequential function local:del($y) { delete node $y/following-sibling::b; };
declare variable $f := local:del#1;
for $y in $d/b return ($f($y), string($y/@k))
