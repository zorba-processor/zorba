(: r:invoke, which is written as an eval, calls a function found by name that inserts a node through a function item: the count of the nodes beside it reads them per tuple and is not hoisted out of the loop. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:call($i) { $f() };
for $i in 1 to 3 return (r:invoke(xs:QName("local:call"), $i), count($d/b))
