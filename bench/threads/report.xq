(: Multithreaded throughput report (bench/threads/run.sh): reads the JSON
 : lines written by mtbench, writes Markdown.
 :
 :   zorba --serialize-text -f -q report.xq -e runs:=runs.jsonl
 :         -e cpus:=$(nproc) [-e base:=other/runs.jsonl]
 :
 : With base, the runs.jsonl of the runs.jsonl of another build, "time / base" is the time
 : per query of this build relative to that build at the same thread count
 : (base qps / qps): the cost of this build where it is above 1. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $runs as xs:string external;
declare variable $base as xs:string external := "";
declare variable $cpus as xs:string external;

declare variable $R := jn:parse-json( file:read-text( $runs ) );
declare variable $B :=
  if ( $base eq "" ) then () else jn:parse-json( file:read-text( $base ) );

declare function local:fmt( $x as xs:double, $digits as xs:integer )
  as xs:string
{
  format-number( $x, if ( $digits eq 3 ) then "0.000" else "0.0" )
};

string-join( (
  if ( exists( $B ) )
  then ( "| query | threads | runs | queries/s | scaling | base queries/s | time / base |",
         "|---|---:|---:|---:|---:|---:|---:|" )
  else ( "| query | threads | runs | queries/s | scaling |",
         "|---|---:|---:|---:|---:|" ),
  for $r in $R
  let $b := $B[ .( "query" ) eq $r( "query" )
                and .( "threads" ) eq $r( "threads" ) ][1]
  return concat(
    "| ", $r( "query" ),
    " | ", $r( "threads" ),
    " | ", $r( "runs" ),
    " | ", local:fmt( $r( "qps" ), 1 ),
    " | ", local:fmt( $r( "scaling" ), 3 ),
    if ( empty( $B ) ) then " |"
    else if ( empty( $b ) ) then " | | |"
    else concat( " | ", local:fmt( $b( "qps" ), 1 ),
                 " | ", local:fmt( $b( "qps" ) div $r( "qps" ), 3 ),
                 " |" ) ),
  "",
  concat( "CPUs available (nproc): ", $cpus, "." ),
  ""
), "&#10;" )
