(: As v03, with the counting query two evals deep: neither eval leaves the
   loop, so each count sees the nodes inserted so far. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare function local:root() { $d };
declare function local:count() { r:eval("r:eval('count(local:root()/b)')") };
for $j in 1 to 3
return (local:add(), local:count())
