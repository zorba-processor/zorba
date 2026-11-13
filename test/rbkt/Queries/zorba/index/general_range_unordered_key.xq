import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

(: A range index without a key type cannot hold a key that has no ordering. :)
m:create(xs:QName("m:y"), <e id="1" v="2000"/>, xs:QName("m:YRange"))
