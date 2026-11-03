(: A join is the same comparison, whichever way the engine evaluates it (F&O 3.0
   op:numeric-equal, XPath 3.0 B.1): 16777217 casts to xs:float(16777216),
   16777218 to itself, 16777219 to xs:float(16777220). Results are pairs of
   positions. The inputs depend on a node so that the join rule can apply. :)
declare variable $d := <d><a>16777217</a><a>16777218</a><a>16777219</a><a>5</a></d>;
let $f := for $x in (16777216, 16777220, 5, 6) return <b>{$x}</b>
return (
  string-join(
    for $a at $p in $d/a
    let $i := xs:integer($a)
    for $b at $q in $f
    let $v := xs:float($b)
    where $i eq $v
    return concat($p, "-", $q), " "),
  "/",
  string-join(
    for $a at $p in $d/a
    let $i := xs:integer($a)
    for $b at $q in $f
    let $v := xs:float($b)
    where $i = $v
    return concat($p, "-", $q), " "),
  "/",
  string-join(
    for $a in $d/a
    let $i := xs:integer($a)
    for $b in $f
    where $i = xs:float($b)
    return concat($i, "-", xs:integer(xs:float($b))), " ")
)
