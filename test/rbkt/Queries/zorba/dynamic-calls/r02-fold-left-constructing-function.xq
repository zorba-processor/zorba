(: No side effect: fn:fold-left with a function that constructs a node gives a new node per evaluation; it is not evaluated once for several iterations. :)
declare function local:mk($a, $b) { <a/> };
let $s := for $i in 1 to 2 return fold-left(1, (), local:mk#2) return $s[1] is $s[2]
