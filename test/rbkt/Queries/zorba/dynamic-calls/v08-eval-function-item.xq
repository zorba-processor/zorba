(: r:eval#1 as a function item, called dynamically, runs a query that inserts a node through a function item: the count of the nodes beside the call reads them per tuple and is not hoisted out of the loop. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:call() { $f() };
declare variable $e := r:eval#1;
for $i in 1 to 3 return ($e("local:call()"), count($d/b))
