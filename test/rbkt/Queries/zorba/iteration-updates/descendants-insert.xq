(: A sequential function called in a let clause inserts a child into the node whose descendants the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/><b k="2"/><b k="3"/></a>;
declare %an:sequential function local:add() { insert node <b k="0"/> as first into $d; };
for $y in $d//b
let $u := local:add()
return string($y/@k)
