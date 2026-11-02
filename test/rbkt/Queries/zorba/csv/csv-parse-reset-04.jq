(: count() and subsequence() over one call site that parses inputs with
 : different headers: each input's header line is not a record. :)
import module namespace csv = "http://zorba.io/modules/json-csv";

string-join(
  for $s in ( "a,b\n1,2\n3,4\n", "x,y,z\n5,6,7\n8,9,10\n" )
  return concat( count( csv:parse( $s ) ), ":",
                 subsequence( csv:parse( $s ), 2 )[1]( "x" ) ),
  "|" )

(: vim:set syntax=xquery et sw=2 ts=2: :)
