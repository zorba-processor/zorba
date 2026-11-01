(: x:parse-html on the kind of report an LLM writes: implied end tags, void
   elements, HTML entities, raw script/style text, a boolean attribute. :)
import module namespace x = "http://zorba.io/modules/xml";

let $doc := x:parse-html(
'<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Q3 Report</title>
<style>td > b { color: red }</style>
<script>if (a < b &amp;&amp; c > 0) { x = "&amp;nbsp;"; }</script>
</head>
<body>
<h1>Q3&amp;nbsp;Report</h1>
<p>Revenue &amp;amp; costs<br>by region<img src="chart.png" alt="chart"></p>
<table class="data">
<thead><tr><th>Region</th><th>Revenue</th></tr></thead>
<tbody>
<tr><td>North</td><td>1,200</td></tr>
<tr><td>South<td>900
</tbody>
</table>
<input type="checkbox" checked>
<p>&amp;copy; 2026 &amp;mdash; done
</body>
</html>')
return
<result>
  <root>{ name($doc/*) }</root>
  <document>{ $doc instance of document-node() }</document>
  <namespace>{ namespace-uri(($doc//table)[1]) }</namespace>
  <title>{ string($doc/html/head/title) }</title>
  <lang>{ string($doc/html/@lang) }</lang>
  <rows>{ count($doc//tr) }</rows>
  <cells>{ count($doc//td) }</cells>
  <south>{ normalize-space($doc//tr[3]/td[2]) }</south>
  <headers>{ string-join($doc//th, "|") }</headers>
  <h1-nbsp>{ string-to-codepoints(string($doc//h1))[3] }</h1-nbsp>
  <amp>{ string(($doc//p)[1]/text()[1]) }</amp>
  <br>{ count($doc//br) }</br>
  <img>{ string($doc//img/@alt) }</img>
  <checked>{ exists($doc//input/@checked) and string($doc//input/@checked) eq "" }</checked>
  <script>{ string($doc//script) }</script>
  <style>{ string($doc//style) }</style>
  <entities>{ string-to-codepoints(substring(normalize-space(($doc//p)[2]), 1, 1)),
              contains(string(($doc//p)[2]), codepoints-to-string(8212)) }</entities>
</result>
