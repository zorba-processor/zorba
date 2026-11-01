(: A malformed document does not abort the iteration: it is returned as an
 : error object (code, namespace, message) next to the parsed ones. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

string-join(
  for $d in dir:documents($root, ("xml", "json", "csv"))
  let $e := $d("error")
  return concat($d("path"), ":",
    if (exists($e))
    then concat("error:", $e("code") ne "", ":", $e("namespace") ne "", ":",
                $e("message") ne "")
    else "ok"),
  " ")
