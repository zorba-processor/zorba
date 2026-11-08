(: As v03, with the counting query two evals deep: each eval query inherits that the query running it hoists nothing, so the innermost count reads the nodes per tuple. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:root() { $d };
for tumbling window $w in r:eval("r:eval('for $j in 1 to 3 return count(local:root()/b)')")
start when true() end when true()
return ($f(), $w)
