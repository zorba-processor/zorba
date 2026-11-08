(: An eval in the body of a user function, reached only through a call of that function, inserts a node: the count of the nodes beside the call reads them per tuple and is not hoisted out of the loop. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:run($i) { r:eval("$f()") };
for $i in 1 to 3 return (local:run($i), count($d/b))
