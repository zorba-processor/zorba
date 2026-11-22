jsoniq version "1.0";
(: Counting the values of a parse builds none of their content at -O1 and -O2:
 : the counts, and the errors with their locations, are those of -O0. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare variable $cases := (
  "{\"a\":1} [1,[2,{\"b\":3}]] {} []",
  "{\"a\":{\"b\":[1,2,{\"c\":null}]}}\n{\"d\":true}",
  "{\"a\":1,\"a\":2}",
  "[{\"x\":{\"y\":1,\"y\":2}}]",
  "{\"a\":[1,2,@]}",
  "{\"a\":1}\n{\"b\":[1,2",
  "" );
for $s at $i in $cases
return {
  "case" : $i,
  "count" :
    try { count(parse-json($s, $options)) }
    catch * { [ local-name-from-QName($err:code), $err:description,
                $err:line-number, $err:column-number ] }
}
