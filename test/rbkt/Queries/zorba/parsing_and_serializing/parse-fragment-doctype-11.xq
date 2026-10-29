(: Quotes and brackets inside DTD comments do not end the DOCTYPE. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

x:parse(
"<!DOCTYPE a [<!-- it's ] here --><!ENTITY e 'y'>]><a>&amp;e;</a>",
<opt:options>
  <opt:parse-external-parsed-entity/>
</opt:options>
)
