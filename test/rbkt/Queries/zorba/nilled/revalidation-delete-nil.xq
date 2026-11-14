import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare revalidation lax;

(: Revalidation after deleting xsi:nil clears the nilled property :)
variable $q := validate { <t:n xsi:nil="true"/> };
delete node $q/@xsi:nil;
nilled($q)
