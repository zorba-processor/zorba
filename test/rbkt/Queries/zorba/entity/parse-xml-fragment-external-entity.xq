(: In an external parsed entity, the relative system identifier of an
 : external entity resolves against the static base URI. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

x:parse(
  "<!DOCTYPE a [<!ENTITY foo SYSTEM 'parse-xml/foo.entity'>]><a>" ||
  codepoints-to-string(38) || "foo;</a><b/>",
  <opt:options>
    <opt:parse-external-parsed-entity/>
  </opt:options>
)
