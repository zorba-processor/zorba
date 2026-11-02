(: CSV files with different headers: the records of each file have the
 : field names of its own header line. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/csv-tree");

string-join(
  for $doc in dir:documents($root, "csv")
  for $row in jn:members($doc("content"))
  return concat($doc("path"), ":",
                string-join(for $k in jn:keys($row)
                            return concat($k, "=", $row($k)), ",")),
  "|")
