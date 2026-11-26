(: An error of the initializer is raised at every level. :)
declare variable $g := (1, 2, fn:error(fn:QName("http://example.org/errors", "E1"), "third item"));

for $x in $g
return $x
