(: Siblings, attributes and text before a node lengthen its node id, so they
   lower the depth at which a constructed tree exceeds the node id limit
   (err:XPDY0130). The limit is reached at the same depth, with the same
   message, whether or not the constructor copies its content (the no-copy
   optimization at O1 and O2). For each shape: the deepest tree that fits, and
   the error for one level more. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer, $level as function(*))
{
  fold-left(1 to $n, "x", $level)
};

declare function local:check($fits as xs:integer, $level as function(*))
{
  string-join((
    string(count(local:nest($fits, $level)/descendant-or-self::node())),
    try { string(count(local:nest($fits + 1, $level)/descendant-or-self::node())) }
    catch * { concat(string($err:code), ": ", $err:description) }
  ), " / ")
};

<result>
  <element-before>{
    local:check(679, function($content, $i) { <a><b/>{ $content }</a> })
  }</element-before>
  <attribute>{
    local:check(679, function($content, $i) { <a x="1">{ $content }</a> })
  }</attribute>
  <text-before>{
    local:check(680, function($content, $i) { <a>t{ $content }</a> })
  }</text-before>
  <eight-elements-before>{
    local:check(254, function($content, $i) {
      <a><b/><b/><b/><b/><b/><b/><b/><b/>{ $content }</a>
    })
  }</eight-elements-before>
  <element-after>{
    local:check(1018, function($content, $i) { <a>{ $content }<b/></a> })
  }</element-after>
</result>
