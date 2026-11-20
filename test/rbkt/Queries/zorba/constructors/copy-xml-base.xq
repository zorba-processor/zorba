(: A copy of an element with an explicit xml:base and another attribute gets
   one hidden base-uri attribute, like the original: nesting 300 such elements,
   each level copied into the next, stays within the node id limit, and the
   base URIs are those of the original. :)
declare function local:nest($n as xs:integer) as element(a)
{
  if ($n eq 1)
  then <a xml:base="http://example.com/" y="1"/>
  else <a xml:base="http://example.com/" y="1">{ local:nest($n - 1) }</a>
};

let $t := local:nest(300)
let $c := copy $x := <r xml:base="http://example.org/" z="1">{ $t/a }</r>
          modify () return $x
return (
  count($t/descendant-or-self::a),
  string(base-uri(($t//a)[last()])),
  count($c//a),
  string(base-uri($c/a)),
  string(base-uri($c))
)
