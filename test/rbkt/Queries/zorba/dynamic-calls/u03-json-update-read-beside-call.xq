(: The same with a JSON array that the sequential function appends to. :)
import module namespace j = "http://jsoniq.org/functions";
declare namespace an = "http://zorba.io/annotations";
declare variable $a := [ ];
declare %an:sequential function local:add() { append json 1 into $a; };
declare variable $f := local:add#0;
for $i in 1 to 3 return ($f(), j:size($a))
