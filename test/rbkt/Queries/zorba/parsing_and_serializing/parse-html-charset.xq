(: A string is already decoded: x:parse-html ignores the charset its meta
   element declares. x:html-doc reads bytes and honors it (the file is
   ISO-8859-1). :)
import module namespace x = "http://zorba.io/modules/xml";

let $s := x:parse-html(
  '<html><head><meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"></head><body><p>caf&#xE9; na&#xEF;ve</p></body></html>')
let $f := x:html-doc("parse-html-latin1.html")
return
<result>
  <string>{ string-to-codepoints(string($s//p)) }</string>
  <file>{ string-to-codepoints(string($f//p)) }</file>
  <file-title>{ string($f//title) }</file-title>
</result>
