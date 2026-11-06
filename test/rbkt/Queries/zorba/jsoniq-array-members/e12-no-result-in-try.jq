jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: try sees the error, with no member before it. :)
try { [ parse-json(file:read-text($d || "bad-tail.json"))[][1] ] } catch * { local-name-from-QName($err:code) }
