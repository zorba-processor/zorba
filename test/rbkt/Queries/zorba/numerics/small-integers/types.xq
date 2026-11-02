(: Small xs:integer values are shared items in the store. Sharing must not
 : leak a type: a value created as xs:integer stays exactly xs:integer, and
 : the same value created as a derived type keeps that type, in whatever
 : order the items are created. :)
let $byte := xs:byte(5)
let $int := xs:int(5)
let $nni := xs:nonNegativeInteger(5)
let $parsed := jn:parse-json("[ 5, -128, 1023, -129, 1024 ]")
let $sum := 2 + 3
return (
  $byte instance of xs:byte,
  $int instance of xs:int and not($int instance of xs:short),
  $nni instance of xs:nonNegativeInteger,
  not(5 instance of xs:long),
  not($sum instance of xs:long),
  for $v in $parsed() return $v instance of xs:integer and not($v instance of xs:long),
  xs:unsignedByte($sum) instance of xs:unsignedByte,
  not($sum instance of xs:unsignedByte),
  $parsed(1) eq $byte,
  $sum eq $parsed(1),
  deep-equal($parsed(), (5, -128, 1023, -129, 1024))
)
