(: doc:put in the modify clause adds a document to the store. :)
import module namespace doc = "http://zorba.io/modules/store/documents";
copy $c := <x/>
modify doc:put("http://zorba.io/tests/transform-targets/doc.xml", document { <a/> })
return $c
