(: A file with an unknown extension is listed without a format and reported
 : as unsupported. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

string-join(
  for $p in ("notes.txt")
  let $d := dir:open($root, $p)
  return concat(
    if ($d("format") instance of xs:string) then $d("format") else "-", ":",
    $d("error")("code"), ":", $d("error")("namespace")),
  " ")
