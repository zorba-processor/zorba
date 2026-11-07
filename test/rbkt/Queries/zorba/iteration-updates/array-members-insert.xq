(: Behavior check (the update runs in a let clause, which already gave this result before the store change): A sequential function called in a let clause inserts a member into the array whose members the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $a := [ 1, 2, 3 ];
declare %an:sequential function local:add() { insert json 0 into $a at position 1; };
for $m in j:members($a)
let $u := local:add()
return $m
