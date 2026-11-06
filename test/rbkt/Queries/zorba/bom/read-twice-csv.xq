import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: The second read also starts after the mark. :)
let $s := file:read-text(concat($d, "bom.csv"))
return (string-join(jn:keys(csv:parse($s)), ","),
        string-join(jn:keys(csv:parse($s)), ","))
