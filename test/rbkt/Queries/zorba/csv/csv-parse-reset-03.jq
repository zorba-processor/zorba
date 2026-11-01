(: Options given to one call of a call site do not carry over to the next
 : call, which uses the defaults: the separator, the field names and the
 : extra-value name of the first call are not reused by the second. :)
import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace jn = "http://jsoniq.org/functions";

string-join(
  for $call in (
    [ "1;2;3\n", { "separator" : ";", "field-names" : [ "p", "q" ],
                   "extra-name" : "e#" } ],
    [ "x,y\n3,4\n", { } ] )
  return string-join(
    for $o in csv:parse( $call[[1]], $call[[2]] )
    return string-join(
      for $k in jn:keys( $o ) return concat( $k, "=", $o( $k ) ), "," ),
    ";" ),
  "|" )

(: vim:set syntax=xquery et sw=2 ts=2: :)
