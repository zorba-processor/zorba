(: A query compiled by eval calls a sequential function through a function item, which inserts a node: the count of the nodes beside the eval reads them per tuple and is not hoisted out of the loop. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
for $i in 1 to 3 return (r:eval("$f()"), count($d/b))
