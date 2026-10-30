(: j0: parse the document and touch the top level (baseline for all other queries) :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

count(jn:keys($data))
