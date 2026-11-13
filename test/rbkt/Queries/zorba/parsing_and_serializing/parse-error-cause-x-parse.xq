(: FODC0006 names the error of the parser and where in the string it is. :)
import module namespace x = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

for $options in (<opt:options/>,
                 <opt:options><opt:parse-external-parsed-entity/></opt:options>)
return
  try { x:parse("<a>
  <b></c>
</a>", $options) }
  catch * {
    string-join((
      string($err:code),
      substring-before(substring-after($err:description, "parse-xml:parse(): ["), "]"),
      string($zerr:data-line-number),
      string($zerr:data-column-number)
    ), " ")
  }
