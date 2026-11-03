import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: UTF-16 with a signature, read twice: ICU takes the signature on each read. :)
let $s := file:read-text(concat($d, "utf16-sig.csv"), "UTF-16")
return (string-join(jn:keys(csv:parse($s)), ","),
        string-join(jn:keys(csv:parse($s)), ","))
