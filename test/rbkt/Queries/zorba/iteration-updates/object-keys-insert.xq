(: Behavior check (the update runs in a let clause, which already gave this result before the store change): A sequential function called in a let clause inserts pairs into the object whose keys the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $o := { "a" : 1, "b" : 2, "c" : 3 };
declare %an:sequential function local:add() { insert json { concat("k", count(j:keys($o))) : 0 } into $o; };
for $k in j:keys($o)
let $u := local:add()
return $k
