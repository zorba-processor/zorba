(: Node ids are at most 255 bytes, which caps a chain of single children at
   1018 levels: fn:doc of nodeid-depth.xml, 1100 levels deep, exceeds an
   implementation limit (err:XPDY0130), whose message names the depth of the
   node that cannot get an id, with the document and where in it that node
   is. Each start tag ends on the next line. :)
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

try { count(doc("nodeid-depth.xml")//a) }
catch * {
  string-join((string($err:code), $err:description,
               string(ends-with($zerr:data-uri, "/doc-limits/nodeid-depth.xml")),
               string($zerr:data-line-number)), " | ")
}
