(: XERCESC-2115: a union keeps the whiteSpace facet of the member type that
   accepts the value instead of always collapsing it. :)
import schema default element namespace "urn:zorba:test:union-whitespace" at "union-whitespace.xsd";

declare function local:show($v as xs:anyAtomicType) as xs:string
{
  typeswitch ($v)
    case xs:integer return "integer:" || $v
    case xs:string return "string:" || string-join(for $c in string-to-codepoints($v) return string($c), ",")
    default return "other"
};

declare variable $values := ("a   ", "   a", "a&#9;b", " 42 ");

string-join((
  for $t in $values
  let $e := validate strict { <item>{$t}</item> }
  return "element " || local:show(data($e)),
  for $t in $values
  let $h := validate strict { <holder v="{$t}"/> }
  return "attribute " || local:show(data($h/@v))
), "|")
