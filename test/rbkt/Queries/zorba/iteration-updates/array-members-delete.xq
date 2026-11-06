(: Behavior check (the update runs in a let clause, which already gave this result before the store change): A sequential function called in a let clause deletes the last member of the array whose members the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $a := [ 1, 2, 3, 4 ];
declare %an:sequential function local:del() { delete json $a(j:size($a)); };
for $m in j:members($a)
let $u := local:del()
return $m
