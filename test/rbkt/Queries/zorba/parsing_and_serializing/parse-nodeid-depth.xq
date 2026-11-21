(: Node ids have no fixed length, so a tree may be as deep as memory and the
   XML parser allow. Chains of 1100 and 10000 nested elements, loaded with
   fn:parse-xml and fn:parse-xml-fragment, are walked, sorted in document
   order and serialized; the 1100-level chains are also compared, copied,
   built into a new element, validated and updated at their deepest element.
   Those walks take stack per level, which 10000 levels may exhaust on a main
   thread under AddressSanitizer; xml_depth_limit tests them at that depth on
   known stacks. Each start tag ends on the next line, so no text node sits
   between the elements. :)
declare function local:nest($n as xs:integer) as xs:string
{
  string-join((for $i in 1 to $n return "<a&#10;>", "x",
               for $i in 1 to $n return "</a>"))
};

declare function local:ids($doc as document-node(), $n as xs:integer)
  as xs:string*
{
  let $deepest := ($doc//a)[last()]
  return (
    string(count($doc//a)),
    string(count($deepest/ancestor::a)),
    string(count(reverse($doc//a)/..)),
    string($doc/a << $deepest),
    string($deepest >> $doc/a),
    string($deepest is $doc/descendant::a[$n]),
    string(string-length(serialize($doc)))
  )
};

declare function local:walks($doc as document-node(), $n as xs:integer)
  as xs:string*
{
  let $updated :=
    copy $d := $doc
    modify (insert node <b/> into ($d//a)[last()],
            insert node <c/> before ($d//a)[last()])
    return $d
  let $last := ($updated//a)[last()]
  return (
    string(deep-equal($doc, parse-xml(local:nest($n)))),
    string(count(document { $doc }//a)),
    string(count(<w>{ $doc/a }</w>//a)),
    string(count((validate lax { $doc })//a)),
    string(count($last/b)),
    string(name($last/preceding-sibling::*[1])),
    string($last/../c << $last/b)
  )
};

declare function local:check($doc as document-node(), $n as xs:integer)
  as xs:string
{
  string-join((local:ids($doc, $n),
               if ($n le 1100) then local:walks($doc, $n) else ()), " ")
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
