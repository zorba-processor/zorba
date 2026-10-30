import module namespace csv = "http://zorba.io/modules/json-csv";

let $text := "sku,qty
A1,2
B7,1"
return [ csv:parse($text) ]
