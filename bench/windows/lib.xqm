jsoniq version "1.0";
(: Records and input sources of the window benchmark (see run.sh).
 : The records are irregular: about 1 in 7 has no "v", 1 in 11 has a null "v",
 : 1 in 13 has a string "v", 1 in 3 has a nested "meta", 1 in 17 an "extra" array.
 : One second separates consecutive records, ten minutes every 100 records. :)
module namespace b = "http://zorba.io/bench/windows";

import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";

declare function b:line($i as integer) as string
{
  "{\"id\":" || $i
  || ",\"ts\":\"" || string(xs:dateTime("2026-01-01T00:00:00Z")
                            + xs:dayTimeDuration("PT1S") * $i
                            + xs:dayTimeDuration("PT10M") * ($i idiv 100))
  || "\",\"sensor\":\"s" || ($i mod 5) || "\""
  || (if ($i mod 11 eq 0) then ",\"v\":null"
      else if ($i mod 13 eq 0) then ",\"v\":\"n/a\""
      else if ($i mod 7 eq 0) then ""
      else ",\"v\":" || ($i mod 100) || ".5")
  || (if ($i mod 3 eq 0) then ",\"meta\":{\"fw\":\"1.2\",\"tags\":[\"a\",\"b\"]}" else "")
  || (if ($i mod 17 eq 0) then ",\"extra\":[1,2,3]" else "")
  || "}"
};

(: gen:N     records made while the query runs; nothing else holds them.
 : lines:F   a JSON Lines file, parsed line by line (file:read-text-lines).
 : whole:F   a JSON Lines file, read into one string, then parse-json.
 : array:F   one JSON array in a file, parse-json, then []. :)
declare %an:nondeterministic function b:source($source as string) as item*
{
  let $kind := substring-before($source, ":")
  let $arg := substring-after($source, ":")
  return
    switch ($kind)
    case "gen" return for $i in 1 to integer($arg) return parse-json(b:line($i))
    case "lines" return file:read-text-lines($arg) ! parse-json($$)
    case "whole" return parse-json(file:read-text($arg))
    case "array" return parse-json(file:read-text($arg))[]
    default return error()
};

declare function b:t($x as item) as xs:dateTime?
{
  xs:dateTime($x.ts)
};

declare function b:numbers($w as item*) as item*
{
  $w.v[$$ instance of decimal]
};
