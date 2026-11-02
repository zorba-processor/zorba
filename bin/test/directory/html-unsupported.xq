xquery version "3.0";

(: Without XML Schema support an HTML file is reported as unsupported. :)
import module namespace dir = "http://zorba.io/modules/directory";

let $d := dir:open($dir:root, "report.html")
return concat("html-", $d("format"), "-", string(exists($d("content"))), "-",
  $d("error")("code"))
