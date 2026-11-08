(: With DTD validation, attributes that share a local name in different
 : namespaces each keep their own value. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

let $e := x:parse("<?xml version='1.0'?>
<!DOCTYPE e [
<!ELEMENT e EMPTY>
<!ENTITY g '+'>
<!ATTLIST e
  xmlns:a CDATA #IMPLIED
  xmlns:b CDATA #IMPLIED
  x CDATA #IMPLIED
  a:x CDATA #IMPLIED
  b:x CDATA #IMPLIED>
]>
<e xmlns:a='A' xmlns:b='B' x='0' a:x='1' b:x='2&amp;g;'/>
",
  <opt:options>
    <opt:DTD-validate/>
  </opt:options>
)/*
return string-join(
  for $a in $e/@*
  order by namespace-uri($a)
  return "Q{" || namespace-uri($a) || "}" || local-name($a) || "=" || string($a),
  " ")
