(: Fixed per-query cost (compilation, module import, serialization) without
 : any file access. :)
import module namespace dir = "http://zorba.io/modules/directory";

count( $dir:root )
