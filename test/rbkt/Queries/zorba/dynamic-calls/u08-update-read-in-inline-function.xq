(: The same with the loop in an inline function called through a function item. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare variable $p := function() { for $j in 1 to 3 return count($d/b) };
for $c in $p() return ($f(), $c)
