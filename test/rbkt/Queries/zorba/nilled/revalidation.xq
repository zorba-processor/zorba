import module namespace schema = "http://zorba.io/modules/schema";
import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare revalidation lax;

(: Validation in place and revalidation after an update set the nilled property :)
variable $n := <t:n xsi:nil="true"/>;
schema:validate-in-place($n);

variable $p := validate { <t:p><t:c>1</t:c></t:p> };
(
  delete node $p/t:c/text(),
  insert node attribute xsi:nil { "true" } into $p/t:c
);

variable $q := validate { <t:n xsi:nil="true"/> };
delete node $q/@xsi:nil;

(nilled($n), nilled($p/t:c), nilled($q))
