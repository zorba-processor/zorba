(:
  Error messages of the schema family: each case raises the error and
  checks its code and the text of its description. A case prints "ok", or
  the code and description it got.
:)
import module namespace zq = "http://zorba.io/modules/zorba-query";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:check(
  $code as xs:QName, $desc as xs:string?,
  $expected as xs:string, $pattern as xs:string) as xs:string
{
  if (local-name-from-QName($code) eq $expected and matches($desc, $pattern))
  then "ok"
  else concat(local-name-from-QName($code), ": ", $desc)
};

declare %an:sequential function local:run(
  $body as xs:string, $expected as xs:string, $pattern as xs:string)
  as xs:string
{
  let $query := concat(
    'import schema namespace s = "urn:zorba-test:schema" at "',
    resolve-uri("schema-ok.xsd", static-base-uri()), '"; ', $body)
  return
    try { zq:evaluate(zq:prepare-main-module($query)) ! "no error" }
    catch * { local:check($err:code, $err:description, $expected, $pattern) }
};

local:run('validate { <s:a><s:b>x</s:b></s:a> }', "XQDY0027",
  "^The query \(.*\) evaluated using zq:evaluate raised an error at line \d+ column \d+: Schema validation failed: value 'x' does not match .+"),

local:run('validate { <s:a><s:c/></s:a> }', "XQDY0027",
  "^The query \(.*\) evaluated using zq:evaluate raised an error at line \d+ column \d+: Schema validation failed: no declaration found for element 's:c'$"),

local:run('validate { <s:z/> }', "XQDY0084",
  "^The query \(.*\) evaluated using zq:evaluate raised an error at line \d+ column \d+: no top-level element declaration in scope for validated element: z @ urn:zorba-test:schema$"),

try {
  zq:prepare-main-module(concat(
    'import schema namespace t = "urn:zorba-test:bad" at "',
    resolve-uri("schema-bad.xsd", static-base-uri()), '"; 1'))
  ! "no error"
}
catch * {
  local:check($err:code, $err:description, "XQST0059",
    "error in schema at line 1, column \d+ of "".*schema-bad\.xsd"": .+$")
}
