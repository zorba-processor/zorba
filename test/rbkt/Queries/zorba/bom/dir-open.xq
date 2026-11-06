import module namespace dir = "http://zorba.io/modules/directory";
declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/bom/data");

(: dir:open parses a file with a byte order mark like any other. :)
let $json := dir:open($root, "bom.json")
let $csv := dir:open($root, "bom.csv")
let $xml := dir:open($root, "bom.xml")
return (
  string-join(for $m in jn:members($json("content")) return string($m), ","),
  string-join(jn:keys(jn:members($csv("content"))[1]), ","),
  $xml("content")/a/string()
)
