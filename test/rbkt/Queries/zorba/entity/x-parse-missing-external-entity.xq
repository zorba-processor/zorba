(: x:parse with an external parsed entity whose file does not exist:
 : FODC0006, instead of a document without the entity. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

x:parse(
  "<!DOCTYPE a [<!ENTITY foo SYSTEM 'parse-xml/missing.entity'>]><a>" ||
  codepoints-to-string(38) || "foo;</a>",
  <opt:options/>
)
