(: Behavior check (the update runs in a let clause, which already gave this result before the store change): A sequential function called in a let clause inserts a last child into the node whose children the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/><b k="2"/><b k="3"/></a>;
declare %an:sequential function local:add() { insert node <b k="4"/> as last into $d; };
for $y in $d/b
let $u := local:add()
return string($y/@k)
