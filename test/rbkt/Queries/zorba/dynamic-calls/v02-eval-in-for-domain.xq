(: A for domain with an eval that can have a side effect gives its whole binding sequence first, as a domain with such a call through a function item does. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; count($d/b) };
declare variable $f := local:add#0;
for $x in (for $j in 1 to 3 return r:eval("$f()")) return ($x, count($d/b))
