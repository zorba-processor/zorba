import schema namespace t = "http://zorba.io/test/revalidation-empty" at "empty.xsd";
declare revalidation strict;

(: Revalidation after deleting the text of an xs:hexBinary element gives it the typed value validation gives an empty one :)
variable $e := validate { <t:h>0A1B</t:h> };
delete node $e/text();
(string($e) eq "", deep-equal(data($e), data(validate { <t:h/> })))
