(: A function item called in the return clause, with the bound key as argument, deletes the last pair of the object whose keys the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $o := { "a" : 1, "b" : 2, "c" : 3, "d" : 4 };
declare %an:sequential function local:del($k) { delete json $o(j:keys($o)[last()]); };
declare variable $f := local:del#1;
for $k in j:keys($o) return ($f($k), $k)
