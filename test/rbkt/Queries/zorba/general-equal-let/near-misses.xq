(: Shapes close to a general = against a LET var that keep comparing each
   pair, with the answers of the nested loop: untyped values of XML (cast to
   the type of the other side, which can fail), a default collation that is
   not the codepoint one, a LET var bound again in an outer loop, a position
   of the var, and != . :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %an:nondeterministic function local:doc()
{
  <d><k>a</k><k>1</k><k>B</k><n>1</n><n>abc</n></d>
};

(: untyped with strings: compared as strings :)
string-join(
  let $keys := local:doc()/k/data()
  for $p in ("a", "1", "b", "B")
  return string($p = $keys),
  " "),

(: untyped with numbers: "a" cannot be cast to xs:double :)
try {
  string-join(
    let $keys := local:doc()/k
    for $p in (1, 1, 2)
    return string($keys = $p),
    " ")
} catch * { local-name-from-QName($err:code) },

(: untyped with untyped :)
string-join(
  let $keys := local:doc()/k/data()
  for $p in local:doc()/n
  return string($p = $keys),
  " "),

(: an outer loop binds the var again :)
string-join(
  for $g in ("a", "B", "1")
  let $keys := local:doc()/k[. ne $g]/string()
  for $p in ("a", "B", "1")
  return string($p = $keys),
  " "),

(: a position of the var :)
string-join(
  let $keys := local:doc()/k/string()
  for $p in ("a", "1", "B")
  return string($p = $keys[2]),
  " "),

(: != :)
string-join(
  let $keys := local:doc()/k/string()
  for $p in ("a", "1", "B")
  return string($p != $keys),
  " ")
