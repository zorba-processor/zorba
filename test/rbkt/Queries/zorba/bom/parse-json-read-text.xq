import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
string-join(
  for $m in jn:members(jn:parse-json(file:read-text(concat($d, "bom.json"))))
  return string($m),
  ",")
