(: Generates small files that expand as much as they can when parsed, for the
 : memory bound of the zorba-mcp document cache (bench/directory/run_cache.sh).
 :
 :   zorba -f -q gen_worst.xq -e out:=DIR -e files:=N -e kb:=K -e shape:=S
 :
 : writes N files of at most K KB each under DIR, all of the shape S:
 :   xml-empty     one element per 4 bytes
 :   xml-attrs     one element with an attribute per 9 bytes
 :   xml-entity-R  an internal entity of 1000 elements, referenced R times
 :                 (the file is about 4 KB; libxml2 refuses too large an
 :                 expansion, and the document then has an error record)
 :   json-arrays   one empty array per 3 bytes
 :   json-objects  one object of one member per 8 bytes
 :   csv-rows      one row of one column per 2 bytes
 :   csv-fields    one row of four empty columns per 4 bytes
 :   html-p        one element per 3 bytes
 :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";

declare variable $out as xs:string external;
declare variable $files as xs:string external;
declare variable $kb as xs:string external;
declare variable $shape as xs:string external;

declare function local:times( $s as xs:string, $n as xs:integer ) as xs:string
{
  string-join( for $i in 1 to $n return $s, "" )
};

declare function local:content( $bytes as xs:integer ) as xs:string
{
  switch ( $shape )
  case "xml-empty" return
    concat( "<a>", local:times( "<b/>", ( $bytes - 7 ) idiv 4 ), "</a>" )
  case "xml-attrs" return
    concat( "<a>", local:times( "<b c=''/>", ( $bytes - 7 ) idiv 9 ), "</a>" )
  case "json-arrays" return
    concat( "[", local:times( "[],", ( $bytes - 4 ) idiv 3 ), "[]]" )
  case "json-objects" return
    concat( "[", local:times( '{"a":0},', ( $bytes - 9 ) idiv 8 ), '{"a":0}]' )
  case "csv-rows" return
    concat( "a&#10;", local:times( "1&#10;", ( $bytes - 2 ) idiv 2 ) )
  case "csv-fields" return
    concat( "a,b,c,d&#10;", local:times( ",,,&#10;", ( $bytes - 8 ) idiv 4 ) )
  case "html-p" return
    concat( "<html><body>", local:times( "<p>", ( $bytes - 12 ) idiv 3 ) )
  default return
    if ( starts-with( $shape, "xml-entity-" ) ) then
      concat( '<!DOCTYPE a [<!ENTITY e "', local:times( "<b/>", 1000 ), '">]><a>',
              local:times( "&amp;e;", xs:integer( substring-after( $shape, "xml-entity-" ) ) ),
              "</a>" )
    else
    error( xs:QName( "local:SHAPE" ), concat( "unknown shape: ", $shape ) )
};

declare %an:sequential function local:write( $path as xs:string,
                                             $content as xs:string )
{
  file:write-text( $path, $content )
};

(
  if ( file:is-directory( $out ) ) then () else file:create-directory( $out ),
  let $content := local:content( xs:integer( $kb ) * 1024 )
  let $ext := substring-before( $shape, "-" )
  for $i in 1 to xs:integer( $files )
  return local:write( concat( $out, "/w", format-integer( $i, "00" ), ".", $ext ),
                      $content ),
  concat( "generated ", $files, " ", $shape, " files of ", $kb, " KB under ", $out )
)
