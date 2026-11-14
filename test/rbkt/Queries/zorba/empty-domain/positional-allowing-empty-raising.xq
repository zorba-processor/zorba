(: A for clause allowing empty over a domain of at most one item that can raise, with a referenced positional variable and the for variable referenced in the where clause: the for clause stays. :)
declare variable $one as xs:integer external := 1;

for $x allowing empty at $i in (6 idiv $one)[. gt 0]
where count($x) eq 1
return ($x, $i)
