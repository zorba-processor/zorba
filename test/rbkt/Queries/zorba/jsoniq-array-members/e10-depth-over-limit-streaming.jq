jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Nesting over the limit, counting the top-level array. :)
head(parse-json(concat("[1,", string-join(for $i in 1 to 1500 return "["), string-join(for $i in 1 to 1500 return "]"), "]"), { "zorba-streaming-members" : true })[])
