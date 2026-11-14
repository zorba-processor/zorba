import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: A valid declared element below an undeclared element gets its type annotation and typed value :)
let $n := (validate lax { <x><y><t:n>12</t:n></y></x> })//t:n
return ($n instance of element(*, xs:integer), data($n) instance of xs:integer)
