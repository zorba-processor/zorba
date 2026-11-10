(: A join returns the inner nodes in the order of the inner domain, with or
   without an index: reverse() gives distinct nodes in reverse document order,
   a path gives them in document order. :)
let $d := <r><b k="1">x</b><b k="1">y</b></r>
return fn:string-join((
  fn:concat("=, reversed domain: ",
    fn:string-join(
      for $a in ("1", "2")
      for $b in fn:reverse($d/b)
      where $b/@k = $a
      return fn:string($b), " ")),
  fn:concat("=, document order domain: ",
    fn:string-join(
      for $a in ("1", "2")
      for $b in $d/b
      where $b/@k = $a
      return fn:string($b), " ")),
  fn:concat("eq, reversed domain: ",
    fn:string-join(
      for $a in ("1", "2")
      for $b in fn:reverse($d/b)
      where fn:string($b/@k) eq $a
      return fn:string($b), " ")),
  fn:concat("eq, document order domain: ",
    fn:string-join(
      for $a in ("1", "2")
      for $b in $d/b
      where fn:string($b/@k) eq $a
      return fn:string($b), " "))),
  "&#10;")
