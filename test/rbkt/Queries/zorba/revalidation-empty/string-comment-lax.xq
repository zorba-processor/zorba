import schema namespace t = "http://zorba.io/test/revalidation-empty" at "empty.xsd";
declare revalidation lax;

(: Revalidation after deleting the text of an xs:string element that keeps a comment child gives it the typed value validation gives it :)
variable $e := validate { <t:s>abc<!--c--></t:s> };
delete node $e/text();
(string($e) eq "", deep-equal(data($e), data(validate { <t:s><!--c--></t:s> })))
