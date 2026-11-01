(: HTML is listed but not parsed until parse-html() exists; a file with an
 : unknown extension is reported the same way. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

string-join(
  for $p in ("report.html", "notes.txt")
  let $d := dir:open($root, $p)
  return concat(
    if ($d("format") instance of xs:string) then $d("format") else "-", ":",
    $d("error")("code"), ":", $d("error")("namespace")),
  " ")
