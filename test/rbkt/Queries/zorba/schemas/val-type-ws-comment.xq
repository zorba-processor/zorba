import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: Collapsing the text around the comment keeps one space between the tokens :)
string(data(validate type xs:token { <e> a <!--c--> b </e> }))
