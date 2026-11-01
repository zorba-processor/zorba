(: Unquoted T and Y are cast to true, F and N to false (documented csv:parse
 : behavior); quoted ones stay strings, and with cast-unquoted-values false
 : all of them stay strings. :)
import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace jn = "http://jsoniq.org/functions";

string-join(
  for $opts in ( { }, { "cast-unquoted-values" : false } )
  let $o := csv:parse( "a,b,c,d,e,f\nT,Y,F,N,\"T\",true\n", $opts )
  return string-join(
    for $k in jn:keys( $o )
    return concat( $k, "=", $o( $k ),
                   if ( $o( $k ) instance of boolean ) then "/b" else "/s" ),
    "," ),
  "|" )

(: vim:set syntax=xquery et sw=2 ts=2: :)
