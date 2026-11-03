import module namespace file = "http://expath.org/ns/file";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: A first character that starts with the same octet as a mark but is not one:
 : U+FF5B and U+FEC0. All of it reaches the parser, which reports its first octet. :)
for $f in ("utf16le-ff5b.json", "utf16le-fec0.json")
return
  try { jn:parse-json(file:read-text(concat($d, $f), "UTF-16LE")) }
  catch * { contains($err:description, "#xEF") }
