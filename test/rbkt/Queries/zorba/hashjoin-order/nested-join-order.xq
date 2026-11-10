(: A join nested in a let returns the inner nodes in the order of the inner
   domain, with or without an index: reverse() gives distinct nodes in
   reverse document order, a path gives them in document order. :)
let $d := <r><a>1</a><a>2</a><b k="1">x</b><b k="1">y</b></r>
return fn:string-join((
  fn:concat("=, reversed domain: ",
    fn:string-join(
      for $a in $d/a
      let $m := for $b in fn:reverse($d/b) where $b/@k = $a return fn:string($b)
      return $m, " ")),
  fn:concat("=, document order domain: ",
    fn:string-join(
      for $a in $d/a
      let $m := for $b in $d/b where $b/@k = $a return fn:string($b)
      return $m, " ")),
  fn:concat("=, reversed domain, two for clauses: ",
    fn:string-join(
      for $a in $d/a
      for $b in fn:reverse($d/b)
      where $b/@k = $a
      return fn:string($b), " ")),
  fn:concat("eq, reversed domain: ",
    fn:string-join(
      for $a in $d/a
      let $m := for $b in fn:reverse($d/b) where fn:string($b/@k) eq fn:string($a) return fn:string($b)
      return $m, " "))),
  "&#10;")
