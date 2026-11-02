(: dir:documents with the "html" format returns the HTML files, parsed or as
 : error records, and nothing else. :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/html-tree");

string-join(
  for $d in dir:documents($root, "html")
  return concat($d("path"), ":", $d("format"), ":",
    if (exists($d("content"))) then "ok" else "error"),
  " ")
