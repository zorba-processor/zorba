(: A where condition that cannot raise becomes an if around the FLWOR, even over a domain that can be empty. :)
declare variable $s as xs:integer* external := ();
declare variable $flag as xs:boolean external := true();

for $x in $s
where $flag
return $x + 1
