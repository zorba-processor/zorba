(: With DTD validation, x:parse reads the document with libxml2's parser,
   whose element nesting is limited to 256 levels: a deeper document exceeds
   an implementation limit, err:XPDY0130, whose message names the limit, with
   where in the string it was exceeded. One element per line after the
   document type declaration. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer) as xs:string
{
  string-join(("<!DOCTYPE a [<!ELEMENT a (a?)>]>",
               for $i in 1 to $n return "<a>",
               for $i in 1 to $n return "</a>"), "&#10;")
};

declare function local:parse($n as xs:integer) as xs:string
{
  try {
    string(count(x:parse(local:nest($n),
                         <opt:options><opt:DTD-validate/></opt:options>)//a))
  }
  catch * {
    string-join((
      string($err:code),
      string(contains($err:description, "element nesting depth of 256")),
      string($zerr:data-line-number gt 256)
    ), " ")
  }
};

<result>
  <shallow>{ local:parse(100) }</shallow>
  <deep>{ local:parse(300) }</deep>
</result>
