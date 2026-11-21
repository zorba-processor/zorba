import schema namespace t = "http://zorba.io/test/val-lax-declared"
  at "val-lax-declared.xsd";

(: Lax validation assesses a declared element strictly: its value must be valid :)
validate lax { <t:n>abc</t:n> }
