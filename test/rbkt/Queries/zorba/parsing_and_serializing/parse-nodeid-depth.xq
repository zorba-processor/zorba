(: Node ids are at most 255 bytes, which caps how deep a tree can be: about a
   thousand levels for a chain of single children, fewer when the elements
   have preceding siblings, here the line breaks. A document nested deeper
   exceeds an implementation limit (err:XPDY0130), whose message names the
   element nesting depth reached, with where in the string it was exceeded.
   One element per line. A document 300 levels deep loads and can be
   walked. :)
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer) as xs:string
{
  string-join((for $i in 1 to $n return "<a>", "x",
               for $i in 1 to $n return "</a>"), "&#10;")
};

declare function local:limit($code, $description, $line) as xs:string
{
  string-join((string($code), $description, string($line)), " | ")
};

let $doc := parse-xml(local:nest(300))
return
<result>
  <shallow>{
    count($doc//a),
    string-length(serialize($doc)),
    normalize-space($doc),
    deep-equal($doc, parse-xml(local:nest(300))),
    count(document { $doc }//a)
  }</shallow>
  <deep>{
    try { count(parse-xml(local:nest(1100))//a) }
    catch * { local:limit($err:code, $err:description, $zerr:data-line-number) }
  }</deep>
  <fragment>{
    try { count(parse-xml-fragment(local:nest(1100))//a) }
    catch * { local:limit($err:code, $err:description, $zerr:data-line-number) }
  }</fragment>
</result>
