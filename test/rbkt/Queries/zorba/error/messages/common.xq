(:
  Error messages of common errors outside the other families: each case
  raises the error and checks its code and the text of its description. A
  case prints "ok", or the code and description it got.
:)
import module namespace refl = "http://zorba.io/modules/reflection";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:check(
  $code as xs:QName, $desc as xs:string?,
  $expected as xs:string, $pattern as xs:string) as xs:string
{
  if (local-name-from-QName($code) eq $expected and matches($desc, $pattern))
  then "ok"
  else concat(local-name-from-QName($code), ": ", $desc)
};

declare function local:eval(
  $query as xs:string, $expected as xs:string, $pattern as xs:string)
  as xs:string
{
  try { refl:eval($query) ! "no error" }
  catch * { local:check($err:code, $err:description, $expected, $pattern) }
};

local:eval('"a" cast as xs:NOTATION', "XPST0080",
  "^""xs:NOTATION"": invalid target type for ""cast"" or ""castable"" expression$"),
local:eval('processing-instruction { "a b" } { "x" }', "XQDY0041",
  "^processing-instruction name cannot be cast to xs:NCName$"),
local:eval('processing-instruction p { "?>" }', "XQDY0026",
  "^computed processing instruction must not contain ""\?>""$"),
local:eval('replace("abc", "x*", "y")', "FORX0003",
  "^""x\*"": regular expression matches the empty string$"),
local:eval('compare("a", "b", "http://example.com/no-such-collation")', "FOCH0002",
  "^""http://example.com/no-such-collation"": unsupported collation")
