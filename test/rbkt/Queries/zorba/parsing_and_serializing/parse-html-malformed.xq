(: x:parse-html recovers from malformed markup: no html/head/body, unclosed
   and misnested elements, stray end tags, unquoted and duplicate attributes,
   and a bare ampersand. :)
import module namespace x = "http://zorba.io/modules/xml";

let $doc := x:parse-html(
'<div id=main class=a class=b><p>one<p>two</span></div>
<b><i>bold-italic</b></i>
<ul><li>a<li>b</ul>
<p>AT&amp;T</p>
<DIV ID="Upper">case</DIV>')
return
<result>
  <root>{ name($doc/*) }</root>
  <body>{ exists($doc/html/body) }</body>
  <paragraphs>{ count($doc//p) }</paragraphs>
  <two>{ string(($doc//p)[2]) }</two>
  <div-class>{ string($doc//div[@id = "main"]/@class) }</div-class>
  <attributes>{ count($doc//div[@id = "main"]/@*) }</attributes>
  <bold>{ string($doc//b) }</bold>
  <items>{ string-join($doc//li ! normalize-space(.), ",") }</items>
  <text>{ string(($doc//p)[3]) }</text>
  <lowercase>{ string($doc//div[@id = "Upper"]) }</lowercase>
</result>
