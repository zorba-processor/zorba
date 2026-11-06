(: As h09, in a function reached only through fn:function-lookup: the decision is the same at every level. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare variable $f := local:bump#0;
declare function local:zero() { 0 };
declare function local:get() { local:zero#0 };
declare function local:h() { for $x in (for $j in 1 to 3 return local:get()() + $g) return ($f(), $x) };
function-lookup(xs:QName("local:h"), 0)()
