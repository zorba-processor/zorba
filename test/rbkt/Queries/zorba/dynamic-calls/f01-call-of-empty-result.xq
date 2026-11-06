(: The function of a dynamic call is one item: an empty one is a type error. :)
declare function local:none($x) { () };
declare variable $fs := (local:none#1, local:none#1);
$fs[1](1)(2)
