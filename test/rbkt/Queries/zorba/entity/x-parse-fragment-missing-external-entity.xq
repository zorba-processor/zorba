(: x:parse of an external parsed entity that refers to an external entity
 : whose file does not exist: FODC0006, instead of nodes without the
 : entity. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

x:parse(
  "<!DOCTYPE a [<!ENTITY foo SYSTEM 'parse-xml/missing.entity'>]><a>" ||
  codepoints-to-string(38) || "foo;</a><b/>",
  <opt:options>
    <opt:parse-external-parsed-entity/>
  </opt:options>
)
