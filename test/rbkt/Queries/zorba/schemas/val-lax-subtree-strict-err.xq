import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: A declared element below an undeclared element is strictly assessed in strict validation :)
validate strict { <t:any><y><t:n>abc</t:n></y></t:any> }
