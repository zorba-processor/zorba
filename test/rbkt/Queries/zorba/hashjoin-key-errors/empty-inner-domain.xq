(: A join over an empty inner domain evaluates no comparison, so the outer
   key, which casts "abc" to xs:integer, is not evaluated, with or without an
   index, whichever operand it is. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

let $doc := <d><x>1</x></d>
return fn:string-join((
  fn:concat("=, inner key first: ",
    try {
      fn:string-join(
        for $a in (<a y="abc"/>, <a y="1"/>)
        let $m := for $b in $doc/none where xs:integer($b) = xs:integer($a/@y) return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("=, outer key first: ",
    try {
      fn:string-join(
        for $a in (<a y="abc"/>, <a y="1"/>)
        let $m := for $b in $doc/none where xs:integer($a/@y) = xs:integer($b) return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq, inner key first: ",
    try {
      fn:string-join(
        for $a in (<a y="abc"/>, <a y="1"/>)
        let $m := for $b in $doc/none where xs:integer($b) eq xs:integer($a/@y) return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq, outer key first: ",
    try {
      fn:string-join(
        for $a in (<a y="abc"/>, <a y="1"/>)
        let $m := for $b in $doc/none where xs:integer($a/@y) eq xs:integer($b) return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) })),
  "&#10;")
