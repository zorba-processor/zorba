(: Full scan: lists and parses every document; the number parsed without
 : error. :)
import module namespace dir = "http://zorba.io/modules/directory";

count( for $d in dir:documents( $dir:root )
       where exists( $d( "content" ) )
       return $d )
