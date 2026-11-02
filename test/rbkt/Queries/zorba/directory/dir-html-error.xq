(: An HTML file that cannot be parsed (nested more than 256 levels deep)
 : yields the per-file error record, as a malformed XML or JSON file does. :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/html-tree");

let $d := dir:open($root, "deep.html")
let $e := $d("error")
return string-join((
  $d("path"), $d("format"),
  string(empty($d("content"))),
  $e("code"), $e("namespace"), string($e("message") ne "")
), " ")
