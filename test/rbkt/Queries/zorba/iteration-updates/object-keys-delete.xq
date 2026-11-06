(: A sequential function called in a let clause deletes the last pair of the object whose keys the for clause iterates. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $o := { "a" : 1, "b" : 2, "c" : 3 };
declare %an:sequential function local:del() { delete json $o(j:keys($o)[last()]); };
for $k in j:keys($o)
let $u := local:del()
return $k
