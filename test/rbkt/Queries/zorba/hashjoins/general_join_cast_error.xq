(: Each join returns what its general comparison returns without an index,
   the error included: an untyped value whose cast to the type of the other
   operand fails, and operands of types that cannot be compared. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

let $doc :=
  <d>
    <u>abc</u><u>2001</u>
    <h>zz</h><h>0A</h>
    <b>a</b><b>AQID</b>
    <y>2001</y><y>2002</y>
    <n>1</n><n>2</n>
    <v>2001</v><v>abc</v>
  </d>
return fn:string-join((
  fn:concat("untyped keys, gYear: ",
    try {
      fn:string-join(
        for $key in (xs:gYear("2001"), xs:gYear("2002"))
        let $m := for $u in $doc/u where $u = $key return $u
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("untyped keys, hexBinary: ",
    try {
      fn:string-join(
        for $key in (xs:hexBinary("0A"), xs:hexBinary("0B"))
        let $m := for $h in $doc/h where $h = $key return $h
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("untyped keys, base64Binary: ",
    try {
      fn:string-join(
        for $key in (xs:base64Binary("AQID"), xs:base64Binary("AQIE"))
        let $m := for $b in $doc/b where $b = $key return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("gYear keys, untyped: ",
    try {
      fn:string-join(
        for $v in $doc/v
        let $m := for $y in $doc/y where xs:gYear($y) = $v return $y
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("integer keys, untyped: ",
    try {
      fn:string-join(
        for $v in $doc/v
        let $m := for $n in $doc/n where xs:integer($n) = $v return $n
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("gYear keys, string: ",
    try {
      fn:string-join(
        for $v in $doc/v
        let $m := for $y in $doc/y where xs:gYear($y) = fn:string($v) return $y
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) })),
  "&#10;")
