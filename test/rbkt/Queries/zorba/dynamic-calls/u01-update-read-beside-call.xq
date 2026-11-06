(: A sequential function called through a function item inserts a node: a count of the nodes beside the call reads them per tuple and is not hoisted out of the loop. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
for $i in 1 to 3 return ($f(), count($d/b))
