import schema namespace t = "http://zorba.io/test/empty-typed-value" at "empty.xsd";
declare revalidation lax;

(: After an update deletes their text, revalidated elements have the typed
   value validation gives the same empty elements. :)
variable $h := validate { <t:h>0A1B</t:h> };
variable $s := validate { <t:s>x</t:s> };
variable $l := validate { <t:l>1 2</t:l> };
variable $c := validate { <t:c>ab</t:c> };
delete nodes ($h, $s, $l, $c)/text();
(
  data($h) instance of xs:hexBinary, data($h) eq xs:hexBinary(""),
  data($s) instance of xs:string,
  empty(data($l)),
  data($c) instance of t:code, string(data($c)) eq ""
)
