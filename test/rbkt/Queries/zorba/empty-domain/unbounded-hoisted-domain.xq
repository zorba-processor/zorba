(: An expr the optimizer moves out of a for loop is not evaluated when the
   loop's domain is empty, as without the move: each expr filters 10^12
   integers and keeps none, so evaluating one does not end in the time the
   test allows. :)
let $doc := <d n="1000000000000"/>
let $n := xs:integer($doc/@n)
return fn:string-join((
  fn:concat("for domain: ",
    fn:count(
      for $a in $doc/none
      for $b in (1 to $n)[. lt 0]
      return ($a, $b))),
  fn:concat("single item: ",
    fn:count(
      for $a in $doc/none
      return ($a, fn:count((1 to $n)[. lt 0]))))),
  "&#10;")
