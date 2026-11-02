(: Schema-validated elements and attributes keep their type annotations and
   typed values: atomic, list and empty content, simple content with an
   attribute (two nodes on one element). :)

import schema namespace p = "http://zorba.io/test/node-packing"
  at "node-packing.xsd";

declare function local:check($name as xs:string, $ok as xs:boolean)
  as xs:string
{
  concat($name, ": ", $ok)
};

let $v := validate { parse-xml('<r xmlns="http://zorba.io/test/node-packing" id="7" ids="1 2 3" note="n"><q>42</q><d u="kg">1.5</d><l>4 5</l><e/></r>') }
let $r := $v/p:r
return string-join((
  local:check("element typed value",
    data($r/p:q) instance of xs:integer and data($r/p:q) eq 42),
  local:check("element type annotation",
    $r/p:q instance of element(p:q, xs:integer) and
    not($r/p:q instance of element(*, xs:untyped))),
  local:check("element string value",
    string($r/p:q) eq "42"),
  local:check("simple content with attribute",
    data($r/p:d) instance of xs:decimal and data($r/p:d) eq 1.5 and
    data($r/p:d/@u) instance of xs:string and string($r/p:d/@u) eq "kg"),
  local:check("list element",
    count(data($r/p:l)) eq 2 and data($r/p:l)[2] instance of xs:integer and
    string($r/p:l) eq "4 5"),
  local:check("empty content",
    empty(data($r/p:e))),
  local:check("attribute typed value",
    data($r/@id) instance of xs:integer and data($r/@id) eq 7),
  local:check("attribute type annotation",
    $r/@id instance of attribute(id, xs:integer)),
  local:check("list attribute",
    count(data($r/@ids)) eq 3 and sum(data($r/@ids)) eq 6 and
    string($r/@ids) eq "1 2 3"),
  local:check("string attribute",
    data($r/@note) instance of xs:string and
    not(data($r/@note) instance of xs:untypedAtomic)),
  local:check("document order",
    $r/@id << $r/p:q and $r/p:q << $r/p:d and
    $r/p:d/@u << $r/p:d/text() and $r/p:l << $r/p:e)
  ),
  "&#10;")
