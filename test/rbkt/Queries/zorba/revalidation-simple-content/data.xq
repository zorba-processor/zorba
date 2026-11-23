import schema namespace t = "http://zorba.io/test/revalidation-simple-content" at "simple-content.xsd";
declare revalidation strict;

(: After revalidation, the typed value of an element of a complex type with
   simple content is a value of the simple type it is derived from, as
   validation gives it: renamed into such types, and with its text replaced. :)
variable $d := validate { document { <t:r><t:b>02</t:b><t:b>03</t:b><t:b>04</t:b><t:i>05</t:i></t:r> } };
rename node $d/t:r/t:b[1] as QName("http://zorba.io/test/revalidation-simple-content", "i");
rename node $d/t:r/t:b[1] as QName("http://zorba.io/test/revalidation-simple-content", "j");
rename node $d/t:r/t:b[1] as QName("http://zorba.io/test/revalidation-simple-content", "s");
replace value of node $d/t:r/t:i[2] with "07";
for $e in $d/t:r/*
return (local-name($e), data($e) instance of xs:integer, data($e) + 1)
