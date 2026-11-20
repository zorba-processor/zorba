(: A document that loads can be validated: the validated copy has the same
   shape as the loaded tree, so its node ids are no longer. The deepest
   loadable chain of single children, 1018 levels, validates whole and from
   its second level, and keeps its base URI. :)
declare function local:nest($n as xs:integer) as xs:string
{
  string-join((for $i in 1 to $n return "<a>", for $i in 1 to $n return "</a>"))
};

let $doc := parse-xml(local:nest(1018))
let $whole := validate lax { $doc }
let $below := validate lax { $doc/a/a }
return
<result>
  <whole>{ count($whole//a) }</whole>
  <below>{ count($below/descendant-or-self::a) }</below>
  <base-uri>{
    base-uri(($whole//a)[last()]) eq base-uri(($doc//a)[last()])
  }</base-uri>
</result>
