(: Paths must stay inside the root: no "..", no absolute paths, no symbolic
 : links (even to a file or directory that exists outside the root). :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

string-join(
  for $p in ("../outside/secret.xml", "orders/../../outside/secret.xml",
             "/etc/passwd", "link-out.xml", "link-dir/secret.xml", "",
             "missing.xml")
  return
    try { dir:open($root, $p)("path") }
    catch * { local-name-from-QName($err:code) },
  " ")
