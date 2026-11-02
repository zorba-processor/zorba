(: A CSV column holding single letters (e.g., D/N for day/night) stays
 : strings in dir:documents: N and Y must not become booleans. Numbers and
 : the words true/false are still cast. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/csv-letters-tree");

string-join(
  for $row in jn:members(dir:open($root, "fires.csv")("content"))
  return string-join(
    for $k in jn:keys($row)
    return concat($k, "=", $row($k),
                  if ($row($k) instance of xs:string) then "/s"
                  else if ($row($k) instance of xs:boolean) then "/b" else "/n"),
    ","),
  "|")
