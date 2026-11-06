(: No side effect, a function that constructs a node called through a function item in the domain of an inner for: the domain runs per outer tuple and gives new nodes; it is not one index built once. :)
declare function local:mk() { <x>1</x> };
declare variable $fs := (local:mk#0, local:mk#0);
declare variable $n := $fs[1];
let $s := for $a in (1, 1) for $b in $n() where $a eq xs:integer($b) return $b
return $s[1] is $s[2]
