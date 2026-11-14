(: FODC0006 names the error of the parser and where in the string it is. :)
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

try { fn:parse-xml-fragment("<a/>
  <b></c>
") }
catch * {
  string-join((
    string($err:code),
    substring-before(substring-after($err:description, "fn:parse-xml-fragment(): ["), "]"),
    string($zerr:data-line-number),
    string($zerr:data-column-number)
  ), " ")
}
