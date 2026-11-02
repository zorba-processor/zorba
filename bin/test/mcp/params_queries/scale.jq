(: Multiplies a number; the factor is optional. :)
declare variable $value as xs:integer external;
declare variable $factor as xs:integer external := 2;
$value * $factor
