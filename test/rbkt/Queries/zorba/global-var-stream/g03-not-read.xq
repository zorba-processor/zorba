(: The body does not read $g; the error of its initializer is still raised. :)
declare variable $g := (1, fn:error(fn:QName("http://example.org/errors", "E1"), "second item"));

for $x in $g
where fn:false()
return $x
