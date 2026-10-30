(: too large for a 64-bit xs:integer: kept exactly, as xs:decimal :)
let $n := jn:parse-json("[ 12345678901234567890 ]")(1)
return ($n, $n instance of xs:decimal)
