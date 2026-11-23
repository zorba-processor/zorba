(: The only reference to $g is the domain of the first for clause of the
 : query body: at -O1 and -O2 its initializer is computed there, streamed. :)
declare variable $g := (1 to 5) ! (. * 2);

for $x in $g
where $x gt 4
return $x
