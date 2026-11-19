(: An eval in the body of a sequential for clause, through a function whose
   body has no variable of the loop in scope (so the eval does not take the
   loop variable along): each iteration runs complete, and the eval, a call
   that can have a side effect, stays in the loop, so each count sees the
   nodes inserted so far. Hoisted out of the loop, the eval would run once,
   before the first insert, and give 0 0 0. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare function local:root() { $d };
declare function local:count() { r:eval("count(local:root()/b)") };
for $j in 1 to 3
return (local:add(), local:count())
