(: Comments and PIs in the internal DTD subset are not fragment nodes. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

x:parse(
"<!DOCTYPE a [<!-- note --><?pi data?><!ENTITY e 'x'>]><a>&amp;e;</a>",
<opt:options>
  <opt:parse-external-parsed-entity/>
</opt:options>
)
