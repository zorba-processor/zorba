(: Listing only: the number of files dir:list reports (no parsing). :)
import module namespace dir = "http://zorba.io/modules/directory";

count( dir:list( $dir:root ) )
