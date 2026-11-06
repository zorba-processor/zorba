(: A function item called in the return clause, with the bound member as argument, deletes the last member of the array whose members the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $a := [ 1, 2, 3, 4 ];
declare %an:sequential function local:del($m) { delete json $a(j:size($a)); };
declare variable $f := local:del#1;
for $m in j:members($a) return ($f($m), $m)
