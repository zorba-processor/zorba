(: Node ids have no fixed length, so a tree may be as deep as memory and the
   XML parser allow. Chains of 1100 and 10000 nested elements, loaded with
   fn:parse-xml and fn:parse-xml-fragment, are walked, sorted in document
   order, compared, serialized, copied, built into a new element, validated
   and updated at their deepest element. Each start tag ends on the next line,
   so no text node sits between the elements. :)
declare function local:nest($n as xs:integer) as xs:string
{
  string-join((for $i in 1 to $n return "<a&#10;>", "x",
               for $i in 1 to $n return "</a>"))
};

declare function local:check($doc as document-node(), $n as xs:integer)
  as xs:string
{
  let $deepest := ($doc//a)[last()]
  let $updated :=
    copy $d := $doc
    modify (insert node <b/> into ($d//a)[last()],
            insert node <c/> before ($d//a)[last()])
    return $d
  let $last := ($updated//a)[last()]
  return string-join((
    string(count($doc//a)),
    string(count($deepest/ancestor::a)),
    string(count(reverse($doc//a)/..)),
    string($doc/a << $deepest),
    string($deepest >> $doc/a),
    string($deepest is $doc/descendant::a[$n]),
    string(string-length(serialize($doc))),
    string(deep-equal($doc, parse-xml(local:nest($n)))),
    string(count(document { $doc }//a)),
    string(count(<w>{ $doc/a }</w>//a)),
    string(count((validate lax { $doc })//a)),
    string(count($last/b)),
    string(name($last/preceding-sibling::*[1])),
    string($last/../c << $last/b)
  ), " ")
};

<result>{
  for $n in (1100, 10000)
  return (
    <xml levels="{$n}">{ local:check(parse-xml(local:nest($n)), $n) }</xml>,
    <fragment levels="{$n}">{
      local:check(parse-xml-fragment(local:nest($n)), $n)
    }</fragment>
  )
}</result>
