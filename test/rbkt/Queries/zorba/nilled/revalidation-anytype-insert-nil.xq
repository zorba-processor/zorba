import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare revalidation lax;

(: Revalidation after inserting xsi:nil into an element of type xs:anyType sets the nilled property :)
variable $a := validate { <t:a/> };
insert node attribute xsi:nil { "true" } into $a;
nilled($a)
