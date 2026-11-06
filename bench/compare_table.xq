(: The comparison table of bench/compare.sh. Run by zorba:
   zorba --serialize-text -f -q compare_table.xq -e raw:=OUTDIR/raw.tsv
         -e base:=LABEL -e head:=LABEL
   Markdown on stdout: one row per query with median and minimum wall time and
   peak RSS of both builds and the ratio head/base. Run 0 (warm-up) is not in
   the statistics, but a failure in any run, run 0 included, is reported. :)
import module namespace file = "http://expath.org/ns/file";

declare namespace an = "http://zorba.io/annotations";

declare variable $raw as xs:string external;
declare variable $base as xs:string external := "base";
declare variable $head as xs:string external := "head";

declare function local:median( $v as xs:double* ) as xs:double? {
  let $s := for $x in $v order by $x return $x
  let $n := count( $s )
  return
    if ( $n = 0 ) then ()
    else if ( $n mod 2 = 1 ) then $s[ ( $n + 1 ) idiv 2 ]
    else ( $s[ $n idiv 2 ] + $s[ $n idiv 2 + 1 ] ) div 2
};

declare function local:sec( $ms as xs:double? ) as xs:string {
  if ( empty( $ms ) ) then "-" else format-number( $ms div 1000, "0.000" )
};

declare function local:ratio( $a as xs:double?, $b as xs:double? ) as xs:string {
  if ( empty( $a ) or empty( $b ) or $b eq 0 ) then "-"
  else format-number( $a div $b, "0.00" ) || "x"
};

declare function local:mib( $kib as xs:double? ) as xs:string {
  if ( empty( $kib ) ) then "-" else format-number( $kib div 1024, "0.0" )
};

declare %an:nondeterministic function local:rows() as element( r )* {
  for $l in file:read-text-lines( $raw )[ normalize-space( . ) ne "" ]
  let $t := tokenize( $l, "&#9;" )
  return
    <r q="{ $t[1] }" b="{ $t[2] }" n="{ $t[3] }" ms="{ $t[4] }" rss="{ $t[5] }"
       st="{ $t[6] }" sum="{ $t[7] }"/>
};

declare function local:failure( $rows as element( r )*, $b as xs:string ) as xs:string? {
  let $bad := $rows[ @b eq $b ][ @st ne "ok" ]
  return
    if ( empty( $bad ) ) then ()
    else $b || ": " || string-join(
      for $r in $bad return $r/@st || " (run " || $r/@n || ")", ", " )
};

declare function local:ok( $rows as element( r )*, $b as xs:string ) as element( r )* {
  $rows[ @b eq $b ][ @st eq "ok" ][ @n ne "0" ]
};

declare function local:out( $rows as element( r )*, $b as xs:string ) as xs:string? {
  ( for $r in local:ok( $rows, $b ) order by xs:integer( $r/@n ) descending return string( $r/@sum ) )[ 1 ]
};

declare function local:cell( $x as xs:string* ) as xs:string {
  "| " || string-join( $x, " | " ) || " |"
};

let $all := local:rows()
let $queries := distinct-values( $all/@q )
let $lines :=
  for $q in $queries
  let $rows := $all[ @q eq $q ]
  let $bok := local:ok( $rows, "base" )
  let $hok := local:ok( $rows, "head" )
  let $bms := $bok ! xs:double( @ms )
  let $hms := $hok ! xs:double( @ms )
  let $bmed := local:median( $bms )
  let $hmed := local:median( $hms )
  let $bmin := min( $bms )
  let $hmin := min( $hms )
  let $brss := max( $bok ! xs:double( @rss ) )
  let $hrss := max( $hok ! xs:double( @rss ) )
  let $fail := ( local:failure( $rows, "base" ), local:failure( $rows, "head" ) )
  let $timed := count( $rows[ @b eq "base" ][ @n ne "0" ] )
  let $partial :=
    for $b in ( "base", "head" )
    let $k := count( local:ok( $rows, $b ) )
    let $all-n := count( $rows[ @b eq $b ][ @n ne "0" ] )
    where $k gt 0 and $k lt max( ( $all-n, $timed ) )
    return $b || " " || $k || " timed runs"
  let $ob := local:out( $rows, "base" )
  let $oh := local:out( $rows, "head" )
  let $out :=
    if ( empty( $ob ) or empty( $oh ) ) then "-"
    else if ( $ob eq $oh ) then "same" else "DIFFERENT"
  let $status :=
    if ( exists( $fail ) ) then "**FAILED** " || string-join( $fail, "; " )
    else if ( exists( $partial ) ) then "ok, " || string-join( $partial, ", " )
    else "ok"
  return
    <l failed="{ exists( $fail ) }" differs="{ $out eq 'DIFFERENT' }">{
      local:cell( ( $q, local:sec( $bmed ), local:sec( $bmin ), local:sec( $hmed ),
                    local:sec( $hmin ), local:ratio( $hmed, $bmed ), local:ratio( $hmin, $bmin ),
                    local:mib( $brss ), local:mib( $hrss ), local:ratio( $hrss, $brss ),
                    $out, $status ) )
    }</l>
let $nfail := count( $lines[ @failed eq "true" ] )
let $ndiff := count( $lines[ @differs eq "true" ] )
let $timed := max( for $q in $queries return count( $all[ @q eq $q ][ @b eq "base" ][ @n ne "0" ] ) )
return string-join( (
  "base: " || $base,
  "",
  "head: " || $head,
  "",
  "Wall time and peak RSS per query, medians and minima over the timed runs " ||
  "(a warm-up run comes first and is not counted). Ratios are head/base: below 1.00x " ||
  "head is faster or smaller.",
  "",
  count( $queries ) || " queries, " || $nfail || " failed, " || $ndiff ||
  " with a different output.",
  "",
  local:cell( ( "query", "base median s", "base min s", "head median s", "head min s",
                "median head/base", "min head/base", "base RSS MiB", "head RSS MiB",
                "RSS head/base", "output", "status" ) ),
  "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---|",
  $lines ! string( . ),
  "" ), "&#10;" )
