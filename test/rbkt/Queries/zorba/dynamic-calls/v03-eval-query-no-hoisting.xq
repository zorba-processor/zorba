(: The query compiled by eval has no call that can have a side effect, but the query that runs it inserts nodes through a function item while it consumes the result lazily: the eval query hoists nothing either, and its count reads the nodes per tuple. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:root() { $d };
let $cs := r:eval("for $j in 1 to 3 return count(local:root()/b)")
for $c in $cs return ($f(), $c)
