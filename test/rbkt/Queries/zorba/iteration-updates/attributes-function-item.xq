(: A function item called in the return clause, with the bound attribute as argument, inserts an attribute into the node whose attributes the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a x="1" y="2" z="3"/>;
declare %an:sequential function local:add($y) { insert node attribute { concat("w", count($d/@*)) } { string($y) } into $d; };
declare variable $f := local:add#1;
for $y in $d/@* return ($f($y), string($y))
