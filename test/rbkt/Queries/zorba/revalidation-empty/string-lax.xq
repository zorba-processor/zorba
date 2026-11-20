import schema namespace t = "http://zorba.io/test/revalidation-empty" at "empty.xsd";
declare revalidation lax;

(: Revalidation after deleting the text of an xs:string element gives it the typed value validation gives an empty one :)
variable $e := validate { <t:s>abc</t:s> };
delete node $e/text();
(string($e) eq "", deep-equal(data($e), data(validate { <t:s/> })))
