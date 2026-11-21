(: Around 31 levels, the node ids of a chain of single children grow past the
   8 bytes kept inline. Document order, identity and duplicate removal hold on
   both sides of that boundary, for parsed and for constructed trees. :)
declare function local:chain($n as xs:integer) as element(a)
{
  if ($n eq 1) then <a/> else <a>{ local:chain($n - 1) }</a>
};

declare function local:check($d as node()) as xs:string
{
  let $s := $d/descendant-or-self::a
  return string-join((
    string(count($s)),
    string(every $i in 1 to count($s) - 1 satisfies $s[$i] << $s[$i + 1]),
    string(every $i in 1 to count($s) - 1 satisfies $s[$i + 1] >> $s[$i]),
    string(every $i in 1 to count($s) - 1 satisfies not($s[$i + 1] << $s[$i])),
    string(every $i in 1 to count($s) - 1 satisfies not($s[$i] is $s[$i + 1])),
    string(every $i in 1 to count($s) - 1
           satisfies exists($s[$i + 1]/ancestor::a[. is $s[$i]])),
    string(count($d//a | $d//a)),
    string(count(($d//a, $d//a) / .))
  ), " ")
};

let $n := 40
let $parsed := parse-xml(string-join((for $i in 1 to $n return "<a>",
                                      for $i in 1 to $n return "</a>")))
return (
  <parsed>{ local:check($parsed) }</parsed>,
  <constructed>{ local:check(document { local:chain($n) }) }</constructed>
)
