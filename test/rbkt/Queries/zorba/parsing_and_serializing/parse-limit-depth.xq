(: Element nesting deeper than the parser's limit (256 levels) exceeds an
   implementation limit: err:XPDY0130, whose message names the limit, with
   where in the string it was exceeded. One element per line. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer) as xs:string
{
  string-join((for $i in 1 to $n return "<a>",
               for $i in 1 to $n return "</a>"), "&#10;")
};

declare function local:outcome($parse as function(xs:string) as node()*)
  as xs:string
{
  try { string(count($parse(local:nest(300))//a)) }
  catch * {
    string-join((
      string($err:code),
      string(contains($err:description, "element nesting depth of 256")),
      string($zerr:data-line-number gt 256)
    ), " ")
  }
};

<result>
  <shallow>{ count(fn:parse-xml(local:nest(100))//a) }</shallow>
  <parse-xml>{ local:outcome(fn:parse-xml#1) }</parse-xml>
  <parse-xml-fragment>{ local:outcome(fn:parse-xml-fragment#1) }</parse-xml-fragment>
  <x-parse>{ local:outcome(x:parse(?, <opt:options/>)) }</x-parse>
</result>
