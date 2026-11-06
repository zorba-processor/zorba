jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: A file that starts with a byte order mark: parse-json ignores the mark (F&O 3.1). :)
[ parse-json(file:read-text($d || "bom.json"))[] ]
