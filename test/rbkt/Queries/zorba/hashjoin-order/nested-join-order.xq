(: A join nested in a let returns the inner nodes in the order of the inner
   domain, with or without an index: reverse() gives distinct nodes in
   reverse document order, a path gives them in document order. :)
declare construction strip;

let $d := <r><a>k</a><a>z</a><b id="1">k</b><b id="2">k</b></r>
return fn:string-join((
  fn:concat("=, reversed domain: ",
    fn:string-join(
      for $a in $d/a
      let $m := for $b in fn:reverse($d/b) where $b = $a return fn:string($b/@id)
      return $m, " ")),
  fn:concat("=, document order domain: ",
    fn:string-join(
      for $a in $d/a
      let $m := for $b in $d/b where $b = $a return fn:string($b/@id)
      return $m, " ")),
  fn:concat("=, reversed domain, two for clauses: ",
    fn:string-join(
      for $a in $d/a
      for $b in fn:reverse($d/b)
      where $b = $a
      return fn:string($b/@id), " ")),
  fn:concat("eq, reversed domain: ",
    fn:string-join(
      for $a in $d/a
      let $m := for $b in fn:reverse($d/b) where fn:string($b) eq fn:string($a) return fn:string($b/@id)
      return $m, " "))),
  "&#10;")
