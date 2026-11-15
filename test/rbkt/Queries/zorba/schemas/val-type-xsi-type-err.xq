import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: xsi:type names a type derived from the stipulated type: it governs :)
validate type xs:decimal {
  <e xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:integer">1.5</e>
}
