(: In a build without XML Schema support an HTML file is listed with its
 : format and reported as unsupported, like a file with an unknown extension. :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

let $d := dir:open($root, "report.html")
return concat($d("format"), ":", $d("error")("code"), ":",
  $d("error")("namespace"))
