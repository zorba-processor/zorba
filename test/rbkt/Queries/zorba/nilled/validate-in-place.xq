import module namespace schema = "http://zorba.io/modules/schema";
import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare revalidation lax;

(: Validation in place sets the nilled property :)
variable $n := <t:n xsi:nil="true"/>;
schema:validate-in-place($n);
nilled($n)
