jsoniq version "1.0";
(:
  Error messages of the JSON and JSONiq family: each case raises the error
  and checks its code and the text of its description. A case prints "ok",
  or the code and description it got.
:)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:check(
  $code as QName, $desc as string?,
  $expected as string, $pattern as string) as string
{
  if (local-name-from-QName($code) eq $expected and matches($desc, $pattern))
  then "ok"
  else concat(local-name-from-QName($code), ": ", $desc)
};

declare variable $key := "a";

[
  try { ({ $key : 1, "a" : 2 }) }
  catch * { local:check($err:code, $err:description, "JNDY0003",
    "^\"a\": duplicate key in object$") },

  try { ({ [ 1 to 3 ] : 1 }) }
  catch * { local:check($err:code, $err:description, "JNTY0004",
    "^cannot atomize an array: an atomic value is expected here$") },

  try { string({ "a" : 1 }) }
  catch * { local:check($err:code, $err:description, "JNTY0024",
    "^an object has no string value$") },

  try { let $o := { "a" : 1 } return $o("a", "b") }
  catch * { local:check($err:code, $err:description, "JNTY0018",
    "^object or array lookup takes zero or one argument$") },

  try { <a>{ { "x" : 1 } }</a> }
  catch * { local:check($err:code, $err:description, "JNTY0011",
    "^object or array cannot appear in the content of a node constructor or updating expression$") },

  try { parse-json("tru") }
  catch * { local:check($err:code, $err:description, "JNDY0021",
    "^illegal JSON literal: expected true, false or null$") },

  try { parse-json("1", { "jsoniq-multiple-top-level-items" : "yes" }) }
  catch * { local:check($err:code, $err:description, "JNTY0020",
    "^\"jsoniq-multiple-top-level-items\" option: xs:string given, xs:boolean expected$") }
]
