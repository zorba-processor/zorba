(: With DTD validation, fn:doc reads the document with libxml2's parser,
   whose element nesting is limited to 256 levels: depth.xml, 300 levels
   deep, exceeds an implementation limit (err:XPDY0130), whose message names
   the limit, with the document and where in it the limit was exceeded. Each
   start tag ends on the next line. :)
declare namespace op = "http://zorba.io/options/features";
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare option op:enable "dtd";

try { count(doc("depth.xml")//a) }
catch * {
  string-join((string($err:code), $err:description,
               string(ends-with($zerr:data-uri, "/doc-limits/depth.xml")),
               string($zerr:data-line-number)), " | ")
}
