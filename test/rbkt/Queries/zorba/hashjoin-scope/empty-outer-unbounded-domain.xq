(: A join under an empty outer domain does not evaluate its inner keys, with
   or without an index: each key filters 10^12 integers and keeps none, so
   evaluating one does not end in the time the test allows. :)
let $doc := <d><b n="1000000000000"/></d>
return fn:string-join((
  fn:concat("eq: ",
    fn:count(
      for $a in $doc/none
      for $b in $doc/b
      where fn:count((1 to xs:integer($b/@n))[. lt 0]) eq xs:integer($a)
      return $b)),
  fn:concat("=: ",
    fn:count(
      for $a in $doc/none
      for $b in $doc/b
      where fn:string(fn:count((1 to xs:integer($b/@n))[. lt 0])) = $a
      return $b))),
  "&#10;")
