(: FODC0006 names the error of the parser. :)
import module namespace x = "http://zorba.io/modules/xml";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

try { x:parse-html(string-join(for $i in 1 to 100000 return "<div>", "")) }
catch * {
  string-join((
    string($err:code),
    substring-before(substring-after($err:description, "x:parse-html(): ["), "]")
  ), " ")
}
