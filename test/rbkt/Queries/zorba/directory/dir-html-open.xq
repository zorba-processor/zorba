(: An HTML file is parsed leniently into a document node: implied end tags
 : are supplied, entities are decoded and the table cells can be queried. :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/html-tree");

let $d := dir:open($root, "pages/table.html")
let $c := $d("content")
return string-join((
  $d("path"),
  $d("format"),
  string(exists($d("error"))),
  string($c instance of document-node()),
  string(count($c//tr)),
  string-join(for $h in $c//th return normalize-space($h), "|"),
  string-join(for $r in $c//tr[td] return normalize-space($r/td[2]), "|"),
  string($c/html/head/title),
  string(contains(string(($c//p)[1]), "Revenue &amp; costs"))
), " ")
