(: A for clause allowing empty over a domain of at most one item that cannot raise, with a referenced positional variable: the position is 1 for an item and 0 for the empty sequence, so the for clause stays. :)
declare variable $opt as xs:integer? external := 5;

for $x allowing empty at $i in $opt
where count($x) eq 1
return ($x, $i)
