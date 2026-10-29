(: A byte order mark without a text declaration is not content, and does
 : not hide a following DOCTYPE. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

count(fn:parse-xml-fragment("&#xFEFF;<a/>")/node()),
x:parse(
"&#xFEFF;<!DOCTYPE a [<!ENTITY e 'z'>]><a>&amp;e;</a>",
<opt:options>
  <opt:parse-external-parsed-entity/>
</opt:options>
)
