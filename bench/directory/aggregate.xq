(: Full scan with a filter and an aggregate over the XML, JSON and CSV
 : records: the records of category c3, counted and summed. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

let $v :=
  for $d in dir:documents( $dir:root )
  let $c := $d( "content" )
  return
    switch ( $d( "format" ) )
    case "xml" return
      for $r in $c/rs/r[ @cat eq "c3" ] return xs:integer( $r/v )
    case "json" return
      for $r in jn:members( $c( "rs" ) )
      where $r( "cat" ) eq "c3"
      return xs:integer( $r( "v" ) )
    case "csv" return
      for $r in jn:members( $c )
      where $r( "cat" ) eq "c3"
      return xs:integer( $r( "v" ) )
    default return ()
return { "matches" : count( $v ), "sum" : sum( $v ) }
