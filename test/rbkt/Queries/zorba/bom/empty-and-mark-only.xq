import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: A file with only a mark has no text, as an empty file. F&O: fn:unparsed-text-lines
 : is tokenize(...)[not(position()=last() and .='')], which gives no lines here. :)
let $m := concat($d, "mark-only.txt")
let $e := concat($d, "empty.txt")
return (
  string-length(file:read-text($m)),
  count(file:read-text-lines($m)),
  count(unparsed-text-lines(file:path-to-uri($m))),
  count(jn:parse-json(file:read-text($m))),
  count(file:read-text-lines($e)),
  count(unparsed-text-lines(file:path-to-uri($e))),
  count(jn:parse-json(file:read-text($e)))
)
