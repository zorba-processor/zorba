(: A let holding an eval that can have a side effect is not eliminated, even if the variable is not used. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
let $x := r:eval("$f()") return count($d/b)
