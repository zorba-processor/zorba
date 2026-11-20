(: The domain of a join that constructs nodes gives new nodes each time it is
   evaluated, once per outer item, with or without an index: directly in the
   domain, or through a let clause between the outer and the inner for clause. :)
let $r := <r><o>1</o><o>1</o></r>
return fn:string-join((
  fn:concat("constructor in the domain, distinct nodes: ",
    fn:count(
      (for $o in $r/o
       for $i in <a><b>1</b></a>/b
       where $i = $o
       return $i) | ())),
  fn:concat("let of a constructor, parent is the let: ",
    fn:string-join(
      for $o in $r/o
      let $d := <a><b>1</b><b>2</b></a>
      for $i in $d/b
      where $i = $o
      return fn:string($i/.. is $d), " "))),
  "&#10;")
