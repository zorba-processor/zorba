(: With DTD validation, FODC0006 names the error of the parser, its message
   and where in the string it is, as without: a well-formedness error and a
   validity error. The column of a validity error differs between libxml2
   versions; only that there is one is checked for it. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

for $doc in ("<!DOCTYPE a [<!ELEMENT a (b)><!ELEMENT b EMPTY>]>
<a>
  <b></c>
</a>",
             "<!DOCTYPE a [<!ELEMENT a (b)><!ELEMENT b EMPTY>]>
<a>
  <c/>
</a>")
return
  try { x:parse($doc, <opt:options><opt:DTD-validate/></opt:options>) }
  catch * {
    string-join((
      string($err:code),
      substring-before(substring-after($err:description, "parse-xml:parse(): ["), "]"),
      string($zerr:data-line-number),
      if (contains($doc, "</c>"))
      then string($zerr:data-column-number)
      else string($zerr:data-column-number gt 0),
      string(contains($err:description, "closing tag mismatch") or
             contains($err:description, '"c": undeclared element'))
    ), " ")
  }
