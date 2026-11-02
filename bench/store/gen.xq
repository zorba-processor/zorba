(: Generates one input file for the store memory benchmark
 : (bench/store/run.sh).
 :
 :   zorba -f -q gen.xq -e out:=FILE -e format:=xml|json|csv -e mb:=M
 :
 : writes about M MiB of records to FILE. Record i only depends on i, so files
 : are reproducible. Each record mixes the shapes the store has to hold:
 :   XML   attributes, short text elements (integer, decimal, name), a longer
 :         text, a nested list of short elements, whitespace between records;
 :   JSON  integers, a decimal, a double, a boolean, a null, short and longer
 :         strings, an array of strings, a nested object, an array of numbers;
 :   CSV   integer, string, integer, decimal, boolean, name and quoted text
 :         columns. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";

declare variable $out as xs:string external;
declare variable $format as xs:string external;
declare variable $mb as xs:string external;

declare variable $chunk := 5000;

declare function local:price( $i as xs:integer ) as xs:string
{
  concat( ( $i * 31 ) mod 1000, ".", format-integer( $i mod 100, "00" ) )
};

declare function local:desc( $i as xs:integer ) as xs:string
{
  concat( "Item ", $i, " description lorem ipsum dolor sit amet" )
};

declare function local:record( $i as xs:integer ) as xs:string
{
  switch ( $format )
  case "xml" return
    concat( '<r id="', $i, '" cat="c', $i mod 10, '"><v>', ( $i * 7919 ) mod 1000,
            '</v><p>', local:price( $i ), '</p><n>name-', $i, '</n><d>',
            local:desc( $i ), '</d><tags><t>t', $i mod 7, '</t><t>t',
            $i mod 13, '</t></tags></r>&#10;' )
  case "json" return
    concat( '{"id":', $i, ',"cat":"c', $i mod 10, '","v":', ( $i * 7919 ) mod 1000,
            ',"p":', local:price( $i ), ',"x":', $i mod 1000, '.5e3,"ok":',
            if ( $i mod 2 eq 0 ) then "true" else "false", ',"z":null,"n":"name-',
            $i, '","d":"', local:desc( $i ), '","tags":["t', $i mod 7, '","t',
            $i mod 13, '"],"pt":{"x":', $i mod 100, ',"y":', $i mod 37,
            '},"nums":[', $i mod 10, ',', $i mod 11, ',', $i mod 12, ',',
            $i mod 13, ']}' )
  default return
    concat( $i, ',c', $i mod 10, ',', ( $i * 7919 ) mod 1000, ',',
            local:price( $i ), ',', if ( $i mod 2 eq 0 ) then "true" else "false",
            ',name-', $i, ',"', local:desc( $i ), '"&#10;' )
};

declare function local:header() as xs:string
{
  switch ( $format )
  case "xml" return "<rs>&#10;"
  case "json" return '{"rs":[&#10;'
  default return "id,cat,v,p,ok,n,d&#10;"
};

declare function local:footer() as xs:string
{
  switch ( $format )
  case "xml" return "</rs>&#10;"
  case "json" return "&#10;]}&#10;"
  default return ""
};

declare function local:sep() as xs:string
{
  if ( $format eq "json" ) then ",&#10;" else ""
};

declare %an:sequential function local:append( $from as xs:integer,
                                              $to as xs:integer )
{
  file:append-text( $out,
    concat( if ( $from gt 1 ) then local:sep() else "",
            string-join( for $i in $from to $to return local:record( $i ),
                         local:sep() ) ) )
};

if ( not( $format = ( "xml", "json", "csv" ) ) ) then
  error( xs:QName( "local:FORMAT" ), concat( "unknown format ", $format ) )
else
  let $target := xs:integer( $mb ) * 1024 * 1024
  (: Average record length over the first records, separator included. :)
  let $avg := string-length( string-join( for $i in 1 to 1000
                                          return local:record( $i ),
                                          local:sep() ) ) div 1000
  let $n := xs:integer( ceiling( $target div $avg ) )
  return (
    file:write-text( $out, local:header() ),
    for $c in 0 to ( $n - 1 ) idiv $chunk
    return local:append( $c * $chunk + 1, min( ( ( $c + 1 ) * $chunk, $n ) ) ),
    file:append-text( $out, local:footer() ),
    concat( "generated ", $n, " ", $format, " records in ", $out )
  )
