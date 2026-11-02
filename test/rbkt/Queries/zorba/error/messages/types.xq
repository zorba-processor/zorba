(:
  Error messages of the casting and types family: each case raises the
  error and checks its code and the text of its description. A case prints
  "ok", or the code and description it got.
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

local:eval('zero-or-one((1, 2))', "FORG0003",
  "^fn:zero-or-one\(\): argument has more than one item$"),
local:eval('one-or-more(())', "FORG0004",
  "^fn:one-or-more\(\): argument is empty$"),
local:eval('exactly-one(())', "FORG0005",
  "^fn:exactly-one\(\): argument does not have exactly one item$"),
local:eval('<r><a/></r>/(a, 1)', "XPTY0018",
  "^last step of a path expression returned both nodes and atomic values$"),
local:eval('(1)[a]', "XPTY0020",
  "^context item of an axis step is not a node$"),
local:eval('xs:yearMonthDuration("P1Y") * xs:double("NaN")', "FOCA0005",
  "^duration cannot be multiplied or divided by NaN$"),
local:eval('dateTime(xs:date("2000-01-01+01:00"), xs:time("10:00:00+02:00"))',
  "FORG0008",
  "^""2000-01-01\+01:00"" and ""10:00:00\+02:00"": date and time have different timezones$"),
local:eval('for $x in 1 group by $k := (1, 2) return $x', "XPTY0004",
  "^grouping key must be at most one atomic value, but atomization produced more than one item$"),
local:eval('"a" treat as xs:integer', "XPDY0050",
  "^xs:string cannot be treated as type xs:integer$"),
local:eval('declare function local:f($x as xs:integer) { $x }; local:f("a")',
  "XPTY0004",
  "^xs:string cannot be promoted to parameter type xs:integer of function local:f\(\)$")
