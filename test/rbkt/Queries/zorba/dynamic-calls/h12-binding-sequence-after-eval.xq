(: As h11, with an eval that compiles another query first: the eval query does not change what this query decided for its functions. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare variable $f := local:bump#0;
declare function local:zero() { 0 };
declare function local:get() { local:zero#0 };
declare function local:h() { for $x in (for $j in 1 to 3 return local:get()() + $g) return ($f(), $x) };
declare variable $fs := (local:h#0, local:h#0);
r:eval("1"), $fs[1]()
