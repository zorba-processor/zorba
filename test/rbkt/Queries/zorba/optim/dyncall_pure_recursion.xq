(: The same with a guard that ends a recursion. :)
declare function local:down($n as xs:integer) { let $r := $fs[1]($n - 1) return if ($n le 0) then 0 else $r };
declare variable $fs := (local:down#1, local:down#1);
local:down(5)
