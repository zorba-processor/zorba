(: The same function, called twice in one query on inputs with different
 : headers, returns each input's own field names. :)
import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace jn = "http://jsoniq.org/functions";

declare function local:parse( $s as string ) as string
{
  string-join(
    for $o in csv:parse( $s )
    return string-join(
      for $k in jn:keys( $o ) return concat( $k, "=", $o( $k ) ), "," ),
    ";" )
};

string-join(
  ( local:parse( "a,b\n1,2\n" ), local:parse( "x,y,z\n3,4,5\n" ) ),
  "|" )

(: vim:set syntax=xquery et sw=2 ts=2: :)
