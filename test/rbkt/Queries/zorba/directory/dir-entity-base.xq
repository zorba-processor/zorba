(: A relative external entity in an XML file is resolved against the URI of
 : the file, not against the base URI of the module: the entity file next to
 : the document is found, by dir:open and by dir:documents. :)
import module namespace dir = "http://zorba.io/modules/directory";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/entity-tree");

string-join((
  string(dir:open($root, "docs/note.xml")("content")),
  for $d in dir:documents($root, "xml")
  where $d("path") eq "docs/note.xml"
  return string($d("content"))
), "|")
