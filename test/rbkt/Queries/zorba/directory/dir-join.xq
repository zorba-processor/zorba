(: One FLWOR over three formats: XML orders joined with JSON payments and
 : CSV customers; documents that failed to parse carry no content. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

let $customers := jn:members(dir:open($root, "customers.csv")("content"))
let $payments :=
  for $d in dir:documents($root, "json")
  let $c := $d("content")
  where exists($c)
  return if ($c instance of object()) then $c else jn:members($c)
return string-join(
  for $doc in dir:documents($root, "xml")
  let $content := $doc("content")
  where exists($content)
  for $o in $content/orders/order
  let $id := string($o/@id)
  for $p in $payments
  where $p("order") eq $id
  for $c in $customers
  where $c("id") eq string($o/@customer)
  order by $id
  return concat($id, "|", $c("name"), "|", $doc("path"), "|",
                $p("status"), "|", $p("amount")),
  ";")
