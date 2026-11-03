(: As float-integer-join.xq, with the operands computed directly from the two for
   variables over nodes, and on xs:anyAtomicType operands (some/every and for with
   where), where the join rule indexes. F&O 3.0 op:numeric-equal, XPath 3.0 B.1. :)
declare variable $d := <d><a>16777217</a><a>16777218</a><a>16777219</a><a>5</a></d>;
declare variable $e := <e><b>16777216</b><b>16777220</b><b>5</b><b>6</b></e>;
declare function local:any($s as xs:string*) as xs:anyAtomicType*
{
  for $x in $s return if (contains($x, "f")) then xs:float(substring-after($x, "f")) else xs:integer($x)
};
let $i := local:any(("16777217", "16777218", "16777219", "5"))
let $f := local:any(("f16777216", "f16777220", "f5", "f6"))
return (
  string-join(for $a at $p in $d/a, $b at $q in $e/b where xs:integer($a) eq xs:float($b) return concat($p, "-", $q), " "),
  "/",
  string-join(for $a at $p in $d/a, $b at $q in $e/b where xs:integer($a) = xs:float($b) return concat($p, "-", $q), " "),
  "/",
  string-join(for $a at $p in $d/a, $b at $q in $e/b where xs:float($a) = xs:integer($b) return concat($p, "-", $q), " "),
  "/",
  string-join(for $x in $i, $y in $f where $x eq $y return concat($x, "-", xs:integer($y)), " "),
  "/",
  string-join(for $y in $f, $x in $i where $y eq $x return concat(xs:integer($y), "-", $x), " "),
  "/",
  every $x in $i[. ne 16777218] satisfies some $y in $f satisfies $x eq $y,
  every $y in $f[. ne 6] satisfies some $x in $i satisfies $y eq $x
)
