import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: A declared element below an element matched by a skip wildcard is not assessed :)
let $n := (validate strict { <t:skip><y><t:n>abc</t:n></y></t:skip> })//t:n
return data($n) instance of xs:untypedAtomic
