import schema namespace t = "http://zorba.io/test/empty-typed-value" at "empty.xsd";

(: An element of a simple type with no text child has the typed value of ""
   for its type: a value of a built-in or user-defined atomic type, () for a
   list type. A comment child does not change that. :)
let $h := data(validate { <t:h/> })
let $s := data(validate { <t:s><!--c--></t:s> })
let $l := data(validate { <t:l/> })
let $c := data(validate { <t:c/> })
return (
  $h instance of xs:hexBinary, $h eq xs:hexBinary(""),
  $s instance of xs:string, $s eq "",
  empty($l),
  $c instance of t:code, $c instance of xs:untypedAtomic, string($c) eq "",
  data(validate { <t:c>ab</t:c> }) instance of t:code,
  data(copy $x := validate { <t:c/> } modify () return $x) instance of t:code
)
