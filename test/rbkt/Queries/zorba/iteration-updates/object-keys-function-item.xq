(: A function item called in the return clause, with the bound key as argument, inserts a pair into the object whose keys the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $o := { "a" : 1, "b" : 2, "c" : 3 };
declare %an:sequential function local:add($k) { insert json { concat($k, "2") : 0 } into $o; };
declare variable $f := local:add#1;
for $k in j:keys($o) return ($f($k), $k)
