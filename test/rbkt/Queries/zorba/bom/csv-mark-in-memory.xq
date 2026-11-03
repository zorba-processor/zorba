import module namespace csv = "http://zorba.io/modules/json-csv";

(: A value that starts with U+FEFF is a string: JSON's mark rule is not CSV's. :)
let $o := { "field-names" : [ "a", "b" ] }
let $t := csv:parse("&#xFEFF;true,1", $o)
let $n := csv:parse("&#xFEFF;null,1", $o)
return ($t("a") instance of xs:string, string-length($t("a")), $t("b"),
        $n("a") instance of xs:string)
