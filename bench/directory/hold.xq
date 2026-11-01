(: Holds every parsed document until the end of the query: its peak RSS
 : minus that of list.xq, over the size of the files, is the in-memory size
 : of the parsed documents per byte of file. :)
import module namespace dir = "http://zorba.io/modules/directory";

let $docs := dir:documents( $dir:root )
return count( $docs ) + count( $docs[ exists( .( "content" ) ) ] )
