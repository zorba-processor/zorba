(: Behavior check (the update runs in a let clause, which already gave this result before the store change): A sequential function called in a let clause inserts a child into the parent of the nodes the following-sibling axis iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/><b k="2"/><b k="3"/></a>;
declare %an:sequential function local:add() { insert node <b k="0"/> as first into $d; };
for $y in $d/b[1]/following-sibling::b
let $u := local:add()
return string($y/@k)
