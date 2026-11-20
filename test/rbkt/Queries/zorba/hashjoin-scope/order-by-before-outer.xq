(: An order by clause between the clause whose variable the inner domain of a
   join depends on and the outer for clause: each tuple joins with the inner
   domain of its own variable, with or without an index. :)
let $docs := (<r n="2"><b id="2x">x</b><b id="2y">y</b></r>,
              <r n="1"><b id="1x">x</b><b id="1y">y</b></r>)
return fn:string-join(
  for $d in $docs
  order by xs:integer($d/@n)
  for $o in ("x", "y")
  for $i in $d/b
  where $i = $o
  return fn:string($i/@id), " ")
