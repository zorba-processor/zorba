(:
  Error messages of the file and I/O family: each case raises the error and
  checks its code and the text of its description. A case prints "ok", or
  the code and description it got.
:)
import module namespace fetch = "http://zorba.io/modules/fetch";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:check(
  $code as xs:QName, $desc as xs:string?,
  $expected as xs:string, $pattern as xs:string) as xs:string
{
  if (local-name-from-QName($code) eq $expected and matches($desc, $pattern))
  then "ok"
  else concat(local-name-from-QName($code), ": ", $desc)
};

declare variable $missing := "file:///zorba-no-such-dir/missing";

(
  try { fn:id("a", <a/>) }
  catch * { local:check($err:code, $err:description, "FODC0001",
    "^fn:id\(\): node is not in a tree whose root is a document node$") },

  try { doc(concat($missing, ".xml")) }
  catch * { local:check($err:code, $err:description, "FODC0002",
    "^""file:///zorba-no-such-dir/missing\.xml"": cannot retrieve resource") },

  try { unparsed-text(concat($missing, ".txt")) }
  catch * { local:check($err:code, $err:description, "FOUT1170",
    "^""file:///zorba-no-such-dir/missing\.txt"": cannot retrieve text resource") },

  try { unparsed-text("file:///zorba-no-such-dir/a.txt#part") }
  catch * { local:check($err:code, $err:description, "FOUT1170",
    "^""file:///zorba-no-such-dir/a\.txt#part"": cannot retrieve text resource: URI must not have a fragment identifier$") },

  try { unparsed-text(static-base-uri(), "no-such-encoding") }
  catch * { local:check($err:code, $err:description, "FOUT1190",
    "^"".+"": cannot decode resource: unsupported encoding ""no-such-encoding""$") },

  try { fetch:content(concat($missing, ".dat")) }
  catch * { local:check($err:code, $err:description, "URI_UNRESOLVED_OR_NOSTREAM",
    "^""file:///zorba-no-such-dir/missing\.dat"": cannot fetch resource: .+") }
)
