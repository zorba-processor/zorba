(: Generates a synthetic document tree for the directory scaling benchmark
 : (.github/workflows/directory-scaling.yml).
 :
 :   zorba -f -q gen_tree.xq -e out:=DIR -e files:=N -e kb:=K
 :
 : writes N files of about K KB each under DIR, in nested directories
 : (DIR/dNN/eN/), cycling through XML, JSON and CSV. Every file holds records
 : with the fields id, cat (c0..c9), v (an integer) and n; the content only
 : depends on N, K and the file number, so trees are reproducible. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";

declare variable $out as xs:string external;
declare variable $files as xs:string external;
declare variable $kb as xs:string external;

declare function local:records( $i as xs:integer, $bytes as xs:integer,
                                $len as xs:integer ) as xs:integer*
{
  let $n := max( ( 1, $bytes idiv $len ) )
  for $k in 1 to $n
  return $i * 100000 + $k
};

declare function local:cat( $id as xs:integer ) as xs:string
{
  concat( "c", $id mod 10 )
};

declare function local:val( $id as xs:integer ) as xs:integer
{
  ( $id * 7919 ) mod 1000
};

declare function local:content( $format as xs:string, $i as xs:integer,
                                $bytes as xs:integer ) as xs:string
{
  switch ( $format )
  case "xml" return
    concat( "<rs>&#10;",
      string-join(
        for $id in local:records( $i, $bytes, 60 )
        return concat( '<r id="', $id, '" cat="', local:cat( $id ), '"><v>',
                       local:val( $id ), '</v><n>name-', $id, '</n></r>&#10;' ),
        "" ),
      "</rs>&#10;" )
  case "json" return
    concat( '{"rs":[&#10;',
      string-join(
        for $id in local:records( $i, $bytes, 55 )
        return concat( '{"id":', $id, ',"cat":"', local:cat( $id ), '","v":',
                       local:val( $id ), ',"n":"name-', $id, '"}' ),
        ",&#10;" ),
      "&#10;]}&#10;" )
  default return
    concat( "id,cat,v,n&#10;",
      string-join(
        for $id in local:records( $i, $bytes, 32 )
        return concat( $id, ",", local:cat( $id ), ",", local:val( $id ),
                       ",name-", $id, "&#10;" ),
        "" ) )
};

declare %an:sequential function local:write( $dir as xs:string,
                                             $path as xs:string,
                                             $content as xs:string )
{
  if ( file:is-directory( $dir ) ) then () else file:create-directory( $dir ),
  file:write-text( $path, $content )
};

(
  let $bytes := xs:integer( $kb ) * 1024
  for $i in 0 to xs:integer( $files ) - 1
  let $format := ( "xml", "json", "csv" )[ $i mod 3 + 1 ]
  let $dir := concat( $out, "/d", format-integer( $i mod 32, "00" ), "/e",
                      ( $i idiv 32 ) mod 8 )
  let $path := concat( $dir, "/f", format-integer( $i, "00000" ), ".",
                       $format )
  return local:write( $dir, $path, local:content( $format, $i, $bytes ) ),
  concat( "generated ", $files, " files of ~", $kb, " KB under ", $out )
)

