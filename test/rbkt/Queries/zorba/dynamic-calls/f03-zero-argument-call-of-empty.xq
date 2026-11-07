(: As f01 with no argument: the function of a dynamic call is one item, and an empty one is a type error. :)
declare function local:none() { () };
declare variable $fs := (local:none#0, local:none#0);
$fs[1]()()
