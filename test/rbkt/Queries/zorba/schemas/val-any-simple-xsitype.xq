import schema namespace t = "http://zorba.io/test/val-lax-declared"
  at "val-lax-declared.xsd";

(: A simple xsi:type on an element declared without a type gives its type annotation and typed value :)
let $u := validate { <t:u xsi:type="xs:integer">12</t:u> }
return ($u instance of element(*, xs:integer), data($u) instance of xs:integer)
