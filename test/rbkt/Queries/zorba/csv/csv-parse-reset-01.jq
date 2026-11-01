(: One call site parses inputs with different headers: each input's own
 : header line gives its field names. :)
import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace jn = "http://jsoniq.org/functions";

declare function local:show( $o as object() ) as string
{
  string-join( for $k in jn:keys( $o ) return concat( $k, "=", $o( $k ) ), "," )
};

string-join(
  for $s in ( "a,b\n1,2\n", "x,y,z\n3,4,5\n" )
  return string-join( for $o in csv:parse( $s ) return local:show( $o ), ";" ),
  "|" )

(: vim:set syntax=xquery et sw=2 ts=2: :)
