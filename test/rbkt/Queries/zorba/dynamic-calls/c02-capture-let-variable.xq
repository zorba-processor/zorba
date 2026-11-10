(: An inline function captures a let variable and is called after the FLWOR
   that bound it has returned it. :)
let $g :=
  let $prefix := "v"
  return function($i) { concat($prefix, $i) }
return ($g(1), for $i in 2 to 3 return $g($i))
