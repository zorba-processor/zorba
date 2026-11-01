import module namespace csv = "http://zorba.io/modules/json-csv";
import module namespace jn = "http://jsoniq.org/functions";

"Q1", for $opts in ( { }, { "cast-unquoted-values" : false } )
      let $o := csv:parse( "a,b,c\nT,Y,F\n", $opts )
      return ( $o, "[", $o( "a" ), "]" ),
"Q2", for $opts in ( { }, { "cast-unquoted-values" : false } )
      let $o := csv:parse( "a,b,c\nT,Y,F\n", $opts )
      for $k in jn:keys( $o )
      return ( $k, "[", $o( $k ), "]" ),
"Q3", for $opts in ( { }, { "cast-unquoted-values" : false } )
      let $o := csv:parse( "a,b,c\nx,Y,F\n", $opts )
      for $k in jn:keys( $o )
      return ( $k, "[", $o( $k ), "]" ),
"Q4", for $opts in ( { "x" : 1 }, { "x" : 2 } )
      let $o := jn:parse-json( "{ \"a\" : \"T\", \"b\" : \"Y\" }" )
      for $k in jn:keys( $o )
      return ( $k, "[", $o( $k ), $opts("x"), "]" ),
"Q5", for $s in ( "a,b\nT,Y\n", "a,b\nU,V\n" )
      let $o := csv:parse( $s, { "cast-unquoted-values" : false } )
      for $k in jn:keys( $o )
      return ( $k, "[", $o( $k ), "]" )
