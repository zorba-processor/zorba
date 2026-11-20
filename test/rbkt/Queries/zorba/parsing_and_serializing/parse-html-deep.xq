(: Element nesting is limited (256 levels, as for XML documents): a deeper
   HTML document exceeds an implementation limit (err:XPDY0130), whose message
   names the limit, with where in the string it was exceeded. One element per
   line; the html and body elements that the parser adds count, so the limit
   is exceeded on line 255. :)
import module namespace x = "http://zorba.io/modules/xml";
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer) as xs:string
{
  string-join(for $i in 1 to $n return "<div>", "&#10;")
};

<result>
  <shallow>{ count(x:parse-html(local:nest(100))//div) }</shallow>
  <deep>{
    try { count(x:parse-html(local:nest(100000))//div) }
    catch * {
      string-join((
        string($err:code),
        string(contains($err:description, "element nesting depth of 256")),
        string($zerr:data-line-number)
      ), " ")
    }
  }</deep>
</result>
