(: A sequential function called in a let clause inserts an attribute into the node whose attributes the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a x="1" y="2" z="3"/>;
declare %an:sequential function local:add() { insert node attribute { concat("w", count($d/@*)) } { "0" } into $d; };
for $y in $d/@*
let $u := local:add()
return string($y)
