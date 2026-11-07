(: A function item called in the return clause, with the bound member as argument, inserts a member into the array whose members the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $a := [ 1, 2, 3 ];
declare %an:sequential function local:add($m) { insert json $m + 10 into $a at position 1; };
declare variable $f := local:add#1;
for $m in j:members($a) return ($f($m), $m)
