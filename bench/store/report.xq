(: Store memory benchmark report (bench/store/run.sh): reads the JSON lines
 : written by storebench and the sizeof() list, writes Markdown.
 :
 :   zorba --serialize-text -f -q report.xq -e runs:=runs.jsonl
 :         -e micro:=micro.jsonl -e sizes:=sizes.txt
 :
 : Every figure is the median over the runs of the same point. The breakdown
 : multiplies the item counts of each parsed file by the per-kind costs that
 : the microbenchmarks measure (differences between documents of N items of
 : one kind each); "unexplained" is the measured heap minus their sum. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $runs as xs:string external;
declare variable $micro as xs:string external;
declare variable $sizes as xs:string external;

declare variable $R := jn:parse-json( file:read-text( $runs ) );
declare variable $M := jn:parse-json( file:read-text( $micro ) );
declare variable $MiB := 1048576;

declare function local:median( $xs as xs:double* ) as xs:double
{
  let $s := for $x in $xs order by $x return $x
  let $n := count( $s )
  return
    if ( $n eq 0 ) then xs:double( "NaN" )
    else if ( $n mod 2 eq 1 ) then $s[ ( $n + 1 ) idiv 2 ]
    else ( $s[ $n idiv 2 ] + $s[ $n idiv 2 + 1 ] ) div 2
};

declare function local:med( $objs as item()*, $key as xs:string )
  as xs:double
{
  local:median( for $o in $objs return xs:double( ( $o( $key ), 0 )[1] ) )
};

declare function local:n( $o as item()?, $key as xs:string ) as xs:double
{
  xs:double( ( $o( $key ), 0 )[1] )
};

declare function local:f( $x as xs:double, $pic as xs:string ) as xs:string
{
  format-number( $x, $pic )
};

(: Per-item heap bytes of a microbenchmark case. :)
declare function local:m( $case as xs:string ) as xs:double
{
  let $c := $M[ .( "case" ) eq $case ]
  return local:med( $c, "heap" ) div local:n( $c[1], "n" )
};

declare function local:m-field( $case as xs:string, $key as xs:string )
  as xs:double
{
  let $c := $M[ .( "case" ) eq $case ]
  return local:med( $c, $key ) div local:n( $c[1], "n" )
};

declare variable $cost-keys := ( "element", "text", "text_char", "attribute",
  "attribute_char", "name", "object", "array", "pair", "csv_pair", "integer", "decimal",
  "double", "boolean", "null", "string", "string_char" );

(: Heap bytes per item of each kind. :)
declare function local:cost( $key as xs:string ) as xs:double
{
  switch ( $key )
  case "element" return local:m( "x_empty" )
  case "text" return local:m( "x_text1" ) - local:m( "x_empty" )
  case "text_char" return
    ( local:m( "x_text64" ) - local:m( "x_text1" ) ) div 63
  case "attribute" return local:m( "x_attr1" ) - local:m( "x_empty" )
  case "attribute_char" return
    ( local:m( "x_attr64" ) - local:m( "x_attr1" ) ) div 63
  case "name" return local:m( "x_names" ) - local:m( "x_empty" )
  case "object" return local:m( "j_obj0" )
  case "array" return local:m( "j_arr0" )
  case "pair" return
    ( local:m( "j_obj4" ) - local:m( "j_obj0" ) ) div 4 - local:m( "j_int" )
  (: A CSV row (c_row4: two integers, two one-character strings) is an
   : object whose keys come from the header line. :)
  case "csv_pair" return
    ( local:m( "c_row4" ) - local:m( "j_obj0" ) - 2 * local:m( "j_int" )
      - 2 * local:m( "j_str1" ) ) div 4
  case "integer" return local:m( "j_int" )
  case "decimal" return local:m( "j_dec" )
  case "double" return local:m( "j_dbl" )
  case "boolean" return local:m( "j_bool" )
  case "null" return local:m( "j_null" )
  case "string" return local:m( "j_str1" )
  case "string_char" return
    ( local:m( "j_str64" ) - local:m( "j_str1" ) ) div 63
  default return error( xs:QName( "local:COST" ), $key )
};

(: Breakdown categories of a parsed file: label, count, cost key. :)
declare function local:categories( $o as item() ) as item()*
{
  if ( $o( "format" ) eq "xml" ) then (
    [ "element nodes", local:n( $o, "elements" ), "element" ],
    [ "text nodes", local:n( $o, "texts" ), "text" ],
    [ "text characters beyond the first",
      local:n( $o, "text_bytes" ) - local:n( $o, "texts" ), "text_char" ],
    [ "attribute nodes", local:n( $o, "attributes" ), "attribute" ],
    [ "attribute characters beyond the first",
      local:n( $o, "attribute_bytes" ) - local:n( $o, "attributes" ),
      "attribute_char" ],
    [ "distinct names", local:n( $o, "distinct_names" ), "name" ]
  ) else (
    [ "objects", local:n( $o, "objects" ), "object" ],
    [ "arrays", local:n( $o, "arrays" ), "array" ],
    [ "object pairs (key, slot)", local:n( $o, "pairs" ),
      if ( $o( "format" ) eq "csv" ) then "csv_pair" else "pair" ],
    [ "integers", local:n( $o, "integers" ), "integer" ],
    [ "decimals", local:n( $o, "decimals" ), "decimal" ],
    [ "doubles", local:n( $o, "doubles" ), "double" ],
    [ "booleans", local:n( $o, "booleans" ), "boolean" ],
    [ "nulls", local:n( $o, "nulls" ), "null" ],
    [ "strings", local:n( $o, "strings" ), "string" ],
    [ "string characters beyond the first",
      local:n( $o, "string_bytes" ) - local:n( $o, "strings" ),
      "string_char" ]
  )
};

declare function local:items( $o as item() ) as xs:double
{
  if ( $o( "format" ) eq "xml" ) then
    local:n( $o, "documents" ) + local:n( $o, "elements" ) +
    local:n( $o, "attributes" ) + local:n( $o, "texts" )
  else
    local:n( $o, "objects" ) + local:n( $o, "arrays" ) +
    local:n( $o, "integers" ) + local:n( $o, "decimals" ) +
    local:n( $o, "doubles" ) + local:n( $o, "booleans" ) +
    local:n( $o, "nulls" ) + local:n( $o, "strings" )
};

declare variable $points :=
  for $f in ( "xml", "json", "csv" )
  for $b in distinct-values( for $o in $R[ .( "format" ) eq $f ]
                             return xs:double( $o( "bytes" ) ) )
  order by $b
  return [ $f, $b ];

declare function local:runs( $p as item() ) as item()*
{
  $R[ .( "format" ) eq $p(1) and xs:double( .( "bytes" ) ) eq $p(2) ]
};

string-join( (
  "## Store memory",
  "",
  concat( "Median of ", count( local:runs( $points[1] ) ),
          " runs per point (one process per run). heap: live malloc bytes ",
          "held by the parsed result (mallinfo2, chunk headers included); ",
          "RSS: resident set after parsing, result held; peak: highest RSS ",
          "while parsing; all three minus the value before parsing. items: ",
          "XML nodes (document, elements, attributes, texts) or JSON items ",
          "(objects, arrays, atomics). mem_size: the store's own estimate ",
          "(Item::mem_size())." ),
  "",
  "| format | file MiB | heap MiB | heap / file | RSS MiB | RSS / file | peak MiB | peak / file | items | heap B / item | allocations / item | mem_size / heap | parse s |",
  "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|",
  for $p in $points
  let $rs := local:runs( $p )
  let $b := $p(2)
  let $heap := local:med( $rs, "heap" )
  let $rss := local:med( $rs, "rss_kb" ) * 1024
  let $hwm := local:med( $rs, "hwm_kb" ) * 1024
  let $items := local:items( $rs[1] )
  return concat( "| ", $p(1), " | ", local:f( $b div $MiB, "0.0" ), " | ",
    local:f( $heap div $MiB, "0.0" ), " | ", local:f( $heap div $b, "0.0" ),
    " | ", local:f( $rss div $MiB, "0.0" ), " | ", local:f( $rss div $b, "0.0" ),
    " | ", local:f( $hwm div $MiB, "0.0" ), " | ", local:f( $hwm div $b, "0.0" ),
    " | ", local:f( $items, "#,##0" ), " | ",
    local:f( $heap div $items, "0.0" ), " | ",
    local:f( local:med( $rs, "news" ) div $items, "0.00" ), " | ",
    local:f( local:med( $rs, "mem_size" ) div $heap, "0.00" ), " | ",
    local:f( local:med( $rs, "ms" ) div 1000, "0.00" ), " |" ),
  "",
  "### Counts (largest file of each format)",
  "",
  for $f in ( "xml", "json", "csv" )
  let $p := $points[ .(1) eq $f ][ last() ]
  let $o := local:runs( $p )[1]
  where exists( $o )
  return (
    concat( "- ", $f, ", ", local:f( $p(2), "#,##0" ), " bytes: ",
      string-join(
        for $k in jn:keys( $o )
        where not( $k = ( "mode", "format", "bytes", "rss_kb", "hwm_kb",
                          "heap", "news", "usable", "ms", "mem_size" ) )
        return concat( $k, " ", local:f( local:n( $o, $k ), "#,##0" ) ),
        ", " ) ) ),
  "",
  "### Microbenchmarks",
  "",
  concat( "Documents of N = ", local:n( $M[1], "n" ),
          " items of one kind, parsed from memory; per item, median of ",
          count( $M[ .( "case" ) eq "x_empty" ] ), " runs. allocations: live ",
          "operator-new blocks; usable: their malloc_usable_size()." ),
  "",
  "| case | heap B / item | allocations / item | usable B / item |",
  "|---|---:|---:|---:|",
  for $c in distinct-values( for $o in $M return $o( "case" ) )
  return concat( "| ", $c, " | ", local:f( local:m( $c ), "0.0" ), " | ",
    local:f( local:m-field( $c, "news" ), "0.00" ), " | ",
    local:f( local:m-field( $c, "usable" ), "0.0" ), " |" ),
  "",
  "Per-kind costs derived from them (bytes):",
  "",
  concat( "| ", string-join( $cost-keys, " | " ), " |" ),
  concat( "|", string-join( for $k in $cost-keys return "---:", "|" ), "|" ),
  concat( "| ", string-join( for $k in $cost-keys
                             return local:f( local:cost( $k ), "0.0" ), " | " ), " |" ),
  "",
  "### Breakdown (largest file of each format)",
  "",
  for $f in ( "xml", "json", "csv" )
  let $p := $points[ .(1) eq $f ][ last() ]
  let $rs := local:runs( $p )
  let $o := $rs[1]
  where exists( $o )
  let $heap := local:med( $rs, "heap" )
  let $cats := local:categories( $o )
  let $sum := sum( for $c in $cats return $c(2) * local:cost( $c(3) ) )
  return (
    concat( "#### ", $f, " (", local:f( $p(2) div $MiB, "0.0" ),
            " MiB file, ", local:f( $heap div $MiB, "0.0" ), " MiB heap)" ),
    "",
    "| category | count | B each | MiB | share of heap |",
    "|---|---:|---:|---:|---:|",
    for $c in $cats
    let $bytes := $c(2) * local:cost( $c(3) )
    order by $bytes descending
    return concat( "| ", $c(1), " | ", local:f( $c(2), "#,##0" ), " | ",
      local:f( local:cost( $c(3) ), "0.0" ), " | ", local:f( $bytes div $MiB, "0.0" ),
      " | ", local:f( 100 * $bytes div $heap, "0" ), "% |" ),
    concat( "| unexplained (heap minus the above) | | | ",
      local:f( ( $heap - $sum ) div $MiB, "0.0" ), " | ",
      local:f( 100 * ( $heap - $sum ) div $heap, "0" ), "% |" ),
    "" ),
  "### sizeof (bytes)",
  "",
  "| class | sizeof |",
  "|---|---:|",
  for $l in tokenize( file:read-text( $sizes ), "&#10;" )[ normalize-space() ]
  let $t := tokenize( $l, " " )
  return concat( "| ", $t[1], " | ", $t[2], " |" ),
  "" ), "&#10;" )
