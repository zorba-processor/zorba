import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: A declared element below an undeclared element is strictly assessed in lax validation :)
validate lax { <x><y><t:n>abc</t:n></y></x> }
