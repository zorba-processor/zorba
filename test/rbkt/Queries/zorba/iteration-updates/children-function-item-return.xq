(: A function item called in the return clause inserts a child into the node whose children the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/><b k="2"/><b k="3"/></a>;
declare %an:sequential function local:add() { insert node <b k="4"/> into $d; };
declare variable $f := local:add#0;
for $y in $d/b return ($f(), string($y/@k))
