xquery version "3.0";

(: Opens the HTML fixture of the shared tree and reads its text. :)
import module namespace dir = "http://zorba.io/modules/directory";

let $d := dir:open($dir:root, "report.html")
return concat("html-", $d("format"), "-",
  string($d("content") instance of document-node()), "-",
  normalize-space(string($d("content")//p)))
