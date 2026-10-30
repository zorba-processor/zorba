(: j2: filter with predicate :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

count($data.orders[][$$.status eq "shipped" and $$.total gt 500])
