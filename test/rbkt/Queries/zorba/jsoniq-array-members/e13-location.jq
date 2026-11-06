jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: The location of the error is the one in the file. :)
try { head(parse-json(file:read-text($d || "bad-token.json"))[]) } catch * { $err:line-number || ":" || $err:column-number }
