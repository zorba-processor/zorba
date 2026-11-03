(: One large file, read whole: what it holds, in the order of the document. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

for $d in dir:documents( $dir:root )
let $c := $d( "content" )
return {
  "path" : $d( "path" ),
  "error" : exists( $d( "error" ) ),
  "count" :
    switch ( $d( "format" ) )
    case "xml" return count( $c//node() )
    case "json" return count( jn:members( $c( "rs" ) ) )
    case "csv" return count( jn:members( $c ) )
    default return count( $c//node() ),
  "sum" :
    switch ( $d( "format" ) )
    case "xml" return sum( for $r in $c/rs/r return xs:integer( $r/v ) )
    case "json" return
      sum( for $r in jn:members( $c( "rs" ) ) return xs:integer( $r( "v" ) ) )
    case "csv" return
      sum( for $r in jn:members( $c ) return xs:integer( $r( "v" ) ) )
    default return 0,
  "first" :
    switch ( $d( "format" ) )
    case "xml" return serialize( ( $c/rs/r )[1] )
    case "json" return serialize( $c( "rs" )( 1 ) )
    case "csv" return serialize( $c( 1 ) )
    default return ""
}
