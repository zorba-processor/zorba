(: doc:remove in the modify clause removes a document from the store. :)
import module namespace doc = "http://zorba.io/modules/store/documents";
doc:put("http://zorba.io/tests/transform-targets/doc.xml", document { <a/> });
copy $c := <x/>
modify doc:remove("http://zorba.io/tests/transform-targets/doc.xml")
return $c
