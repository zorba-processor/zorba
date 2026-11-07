(: The same when the count is in a function item that the compiler can follow. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare variable $p := function() { count($d/b) };
for $i in 1 to 3 return ($f(), $p())
