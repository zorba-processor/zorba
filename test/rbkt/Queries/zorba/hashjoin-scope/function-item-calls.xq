(: Calls of a function item that captures a variable run one inside the other
   (a recursion between two outer items) or interleaved (two lazily consumed
   results): each call joins with the inner domain of its own argument, with or
   without an index. :)
let $docs := (<r><b id="1x">x</b><b id="1y">y</b></r>,
              <r><b id="2x">x</b><b id="2y">y</b></r>)
let $f := function($self, $n as xs:integer) as xs:string* {
  for $d in $docs[$n]
  for $o in ("x", "y")
  let $rest := if ($o eq "x") then $self($self, $n + 1) else ()
  for $i in $d/b
  where $i = $o
  return (fn:string($i/@id), $rest)
}
let $g := function($n as xs:integer) as xs:string* {
  for $d in $docs[$n]
  for $o in ("x", "y")
  for $i in $d/b
  where $i = $o
  return fn:string($i/@id)
}
return fn:string-join((
  fn:concat("recursive: ", fn:string-join($f($f, 1), " ")),
  fn:concat("interleaved: ",
    fn:string-join(for $a in $g(1) for $b in $g(2) return fn:concat($a, $b), " "))),
  "&#10;")
