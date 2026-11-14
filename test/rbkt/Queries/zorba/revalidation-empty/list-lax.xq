import schema namespace t = "http://zorba.io/test/revalidation-empty" at "empty.xsd";
declare revalidation lax;

(: Revalidation after deleting the text of an element of a list type without minLength gives it the typed value validation gives an empty one :)
variable $e := validate { <t:l>1 2 3</t:l> };
delete node $e/text();
(string($e) eq "", deep-equal(data($e), data(validate { <t:l/> })))
