import module namespace jx = "http://zorba.io/modules/json-xml";

(: A number element whose value starts with U+FEFF is not a number. :)
jx:xml-to-json(
  <sn:json xmlns:sn="http://john.snelson.org.uk/parsing-json-into-xquery" type="object">
    <sn:pair name="a" type="number">&#xFEFF;12</sn:pair>
  </sn:json>)
