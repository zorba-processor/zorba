(: A value parsed after an unquoted boolean can be used as an object key. :)
import module namespace csv = "http://zorba.io/modules/json-csv";

let $o := csv:parse( "a,b\ntrue,k1\n" )
let $j := { $o( "b" ) : 42 }
return ( $o( "a" ), $j( "k1" ) )

(: vim:set syntax=xquery et sw=2 ts=2: :)
