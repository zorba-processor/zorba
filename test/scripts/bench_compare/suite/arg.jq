declare variable $n as xs:string external := "unset";
if ( $n eq "41" ) then 42 else fn:error( xs:QName( "local:no-arg" ), "n is " || $n )
