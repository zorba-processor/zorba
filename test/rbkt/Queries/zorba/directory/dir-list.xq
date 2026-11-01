(: Recursive listing: relative paths in order, format by extension, size and
 : modification time; hidden entries and symbolic links are skipped. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

string-join(
  for $d in dir:list($root)
  return concat(
    $d("path"), ":",
    if ($d("format") instance of xs:string) then $d("format") else "-", ":",
    $d("size") instance of xs:integer, ":",
    $d("modified") castable as xs:dateTime),
  " ")
