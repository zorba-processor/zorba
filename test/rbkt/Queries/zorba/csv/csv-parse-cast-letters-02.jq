(: With cast-letter-booleans false, unquoted T/Y/F/N (and single letters in
 : general) stay strings, while true/false words, numbers and null are still
 : cast. The default is unchanged. :)
import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace jn = "http://jsoniq.org/functions";

string-join(
  for $opts in ( { }, { "cast-letter-booleans" : true },
                 { "cast-letter-booleans" : false } )
  let $o := csv:parse( "a,b,c,d,e,f,g,h\nT,Y,F,N,true,false,12,null\n", $opts )
  return string-join(
    for $k in jn:keys( $o )
    return concat( $k, "=", $o( $k ),
                   if ( $o( $k ) instance of boolean ) then "/b"
                   else if ( $o( $k ) instance of xs:integer ) then "/i"
                   else if ( $o( $k ) instance of null ) then "/n"
                   else "/s" ),
    "," ),
  "|" )
