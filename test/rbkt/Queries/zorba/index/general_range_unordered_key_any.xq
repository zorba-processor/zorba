import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

(: Nor can one whose key type is xs:anyAtomicType*. :)
m:create(xs:QName("m:y"), <e id="1" v="2000"/>, xs:QName("m:YAnyRange"))
