(: Typed values, string values, identity and document order of untyped
   element and attribute nodes with zero, one, two and more attributes and
   children, built by the XML parser and by element constructors. :)

declare function local:check($name as xs:string, $ok as xs:boolean)
  as xs:string
{
  concat($name, ": ", $ok)
};

let $xml := '<r a="1" b="two"><e/><s>x</s><t k="v">y</t><u>1<!--c-->2</u><m><i/><i/><i/></m></r>'
let $parsed := parse-xml($xml)/r
let $constructed := <r a="1" b="two"><e/><s>x</s><t k="v">y</t><u>1<!--c-->2</u><m><i/><i/><i/></m></r>
return string-join(
  for $r at $n in ($parsed, $constructed)
  let $p := if ($n eq 1) then "parsed " else "constructed "
  return (
    local:check(concat($p, "attribute typed value"),
      data($r/@a) instance of xs:untypedAtomic and data($r/@a) eq "1"),
    local:check(concat($p, "attribute string value"),
      string($r/@b) eq "two"),
    local:check(concat($p, "attribute type annotation"),
      $r/@a instance of attribute(a, xs:untypedAtomic)),
    local:check(concat($p, "element type annotation"),
      $r/s instance of element(s, xs:untyped)),
    local:check(concat($p, "single child typed value"),
      data($r/s) instance of xs:untypedAtomic and data($r/s) eq "x"),
    local:check(concat($p, "attribute and child"),
      string($r/t/@k) eq "v" and string($r/t) eq "y" and
      count($r/t/node()) eq 1 and count($r/t/@*) eq 1),
    local:check(concat($p, "empty element"),
      empty($r/e/node()) and string($r/e) eq "" and
      data($r/e) instance of xs:untypedAtomic),
    local:check(concat($p, "mixed string value"),
      string($r/u) eq "12" and count($r/u/node()) eq 3),
    local:check(concat($p, "three children"),
      count($r/m/i) eq 3 and count($r/descendant::node()) eq 13),
    local:check(concat($p, "document order"),
      $r/@a << $r/@b and $r/@b << $r/e and $r/e << $r/s and
      $r/s << $r/s/text() and $r/t/@k << $r/t/text() and
      $r/m << $r/m/i[1] and $r/m/i[1] << $r/m/i[3]),
    local:check(concat($p, "identity"),
      $r/s/text()/.. is $r/s and $r/t/@k/.. is $r/t and
      $r/m/i[2]/following-sibling::i is $r/m/i[3] and
      $r/m/i[3]/preceding-sibling::i[1] is $r/m/i[2])
  ),
  "&#10;")
