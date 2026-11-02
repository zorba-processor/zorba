(:
  Error messages of the modules and imports family: each case compiles a
  query that raises the error and checks its code and the text of its
  description. A case prints "ok", or the code and description it got.
:)
import module namespace zq = "http://zorba.io/modules/zorba-query";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:check(
  $code as xs:QName, $desc as xs:string?,
  $expected as xs:string, $pattern as xs:string) as xs:string
{
  if (local-name-from-QName($code) eq $expected and matches($desc, $pattern))
  then "ok"
  else concat(local-name-from-QName($code), ": ", $desc)
};

declare function local:at($file as xs:string) as xs:string
{
  resolve-uri($file, static-base-uri())
};

declare function local:compile(
  $query as xs:string, $expected as xs:string, $pattern as xs:string)
  as xs:string
{
  try { if (exists(zq:prepare-main-module($query))) then "no error" else "" }
  catch * { local:check($err:code, $err:description, $expected, $pattern) }
};

local:compile(
  concat('import module namespace m = "urn:zorba-test:missing" at "',
         local:at("no-such-module.xqlib"), '"; 1'),
  "XQST0059",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: module ""urn:zorba-test:missing"" not found at "".*no-such-module\.xqlib""$"),

local:compile(
  concat('import module namespace m = "urn:zorba-test:main" at "',
         local:at("not-library.xqlib"), '"; 1'),
  "XQST0059",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: module ""urn:zorba-test:main"" at "".*not-library\.xqlib"" is not a library module: it must start with a module namespace declaration$"),

local:compile(
  concat('import module namespace m = "urn:zorba-test:expected" at "',
         local:at("other-namespace.xqlib"), '"; 1'),
  "XQST0059",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: module at "".*other-namespace\.xqlib"" has target namespace ""urn:zorba-test:other"", not ""urn:zorba-test:expected""$"),

local:compile(
  concat('import module namespace w = "urn:zorba-test:wrong" at "',
         local:at("wrong-namespace.xqlib"), '"; 1'),
  "XQST0048",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: ""o:f"": not in the target namespace of the library module; use the prefix of its module declaration$"),

local:compile(
  'declare function f() { 1 }; f()',
  "XQST0045",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: ""f"": function name in reserved namespace ""http://www.w3.org/2005/xpath-functions""; use another prefix \(local: in a main module\)$"),

local:compile(
  'declare namespace xml = "urn:x"; 1',
  "XQST0070",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: ""xml"": reserved prefix cannot be declared$"),

local:compile(
  'import module namespace a = "http://zorba.io/modules/reflection";
   import module namespace b = "http://zorba.io/modules/reflection"; 1',
  "XQST0047",
  "^The query compiled using zq:prepare-main-module raised an error at .* column \d+: ""http://zorba.io/modules/reflection"": module namespace imported more than once$")
