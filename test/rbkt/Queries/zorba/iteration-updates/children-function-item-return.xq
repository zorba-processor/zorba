(: A function item called in the return clause, with the bound child as argument, inserts a child into the node whose children the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a><b k="1"/><b k="2"/><b k="3"/></a>;
declare %an:sequential function local:add($y) { insert node <b k="{$y/@k + 3}"/> into $d; };
declare variable $f := local:add#1;
for $y in $d/b return ($f($y), string($y/@k))
