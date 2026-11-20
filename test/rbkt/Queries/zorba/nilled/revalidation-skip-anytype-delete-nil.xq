import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare revalidation skip;

(: Deleting xsi:nil from an element of type xs:anyType clears the nilled property :)
variable $a := validate { <t:a xsi:nil="true"/> };
delete node $a/@xsi:nil;
nilled($a)
