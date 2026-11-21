xquery version "3.0";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: Inserting an attribute that the element already has fails only after all
   the insertions were applied (XUDY0021), so they are undone: the element and
   the inserted nodes are as before, also where the inserted text had merged
   with the element's text and where an inserted subtree had an xml:base. :)
variable $t := <a xml:base="http://example.com/x/" k="0"><b/>t</a>;
variable $c := <c xml:base="s/" k="1"><d>e</d></c>;
variable $code := "no error";
try
{
  (insert node attribute k { 1 } into $t,
   insert nodes (text { "u" }, $c) into $t,
   insert node <e/> before $t/b);
}
catch *
{
  $code := local-name-from-QName($err:code);
}
(
  $code,
  serialize($t),
  count($t/node()),
  string(base-uri($t/b)),
  serialize($c),
  string(base-uri($c/d)) eq string(base-uri($c))
)
