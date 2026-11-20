import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The whitespace facet applies to the text around the comment as one value :)
validate strict { <t:i>4 <!--c--> 2</t:i> }
