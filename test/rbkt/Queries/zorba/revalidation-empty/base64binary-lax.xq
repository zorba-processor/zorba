import schema namespace t = "http://zorba.io/test/revalidation-empty" at "empty.xsd";
declare revalidation lax;

(: Revalidation after deleting the text of an xs:base64Binary element gives it the typed value validation gives an empty one :)
variable $e := validate { <t:b>AAEC</t:b> };
delete node $e/text();
(string($e) eq "", deep-equal(data($e), data(validate { <t:b/> })))
