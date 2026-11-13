(: A join under an empty outer domain does not evaluate its inner domain, with
   or without an index: the domain filters 10^12 integers and returns none, so
   evaluating it does not end in the time the test allows. :)
let $doc := <d n="1000000000000"/>
let $n := xs:integer($doc/@n)
return fn:string-join((
  fn:concat("eq: ",
    fn:count(
      for $a in $doc/none
      for $b in (1 to $n)[. lt 0]
      where $b eq xs:integer($a)
      return $b)),
  fn:concat("=: ",
    fn:count(
      for $a in $doc/none
      for $b in $doc/descendant-or-self::*[(1 to $n)[. lt 0]]
      where $b = $a
      return $b))),
  "&#10;")
