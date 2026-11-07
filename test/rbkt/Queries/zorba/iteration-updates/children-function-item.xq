(: The children of $d/b are iterated while a function item inserts children into $d: each evaluation of $d/b returns the children $d had when it began. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/></a>;
declare %an:sequential function local:add() { insert node (<b k="2"/>, <b k="3"/>) into $d; };
declare variable $f := local:add#0;
declare function local:j() { for $j in 1 to 3 for $y in $d/b where xs:integer($y/@k) eq $j return $j };
for $c in local:j() return ($f(), $c)
