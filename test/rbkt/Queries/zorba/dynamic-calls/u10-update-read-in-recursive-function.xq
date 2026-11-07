(: The same as u07 with a recursive function, which is not inlined into the query body. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:counts($r as xs:boolean) { if ($r) then for $j in 1 to 3 return count($d/b) else local:counts(true()) };
for $c in local:counts(false()) return ($f(), $c)
