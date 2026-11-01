(: Element nesting is limited (256 levels, as for XML documents): a deeper
   HTML document is an error, not a crash. :)
import module namespace x = "http://zorba.io/modules/xml";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer) as xs:string
{
  string-join(for $i in 1 to $n return "<div>", "")
};

<result>
  <shallow>{ count(x:parse-html(local:nest(100))//div) }</shallow>
  <deep>{
    try { count(x:parse-html(local:nest(100000))//div) }
    catch err:FODC0006 { "FODC0006" }
  }</deep>
</result>
