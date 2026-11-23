import module namespace p = "http://zorba.io/modules/xml";
import schema namespace opt = "http://zorba.io/modules/xml-options";

(: A string is already decoded: the encoding its XML or text declaration
   names does not apply (F&O 3.1, 14.7.1, Notes). Each é stays one character,
   233, instead of being decoded again from its UTF-8 bytes (195 169). :)
(
  string-to-codepoints(fn:parse-xml(
    "<?xml version='1.0' encoding='ISO-8859-1'?><a>&#xE9;</a>")),
  string-to-codepoints(fn:parse-xml-fragment(
    "<?xml encoding='ISO-8859-1'?><a>&#xE9;</a>&#xE9;")),
  string-to-codepoints(string-join(p:parse(
    "<?xml version='1.0' encoding='ISO-8859-1'?><a>&#xE9;</a>",
    <opt:options/>))),
  string-to-codepoints(string-join(p:parse(
    "<?xml encoding='ISO-8859-1'?><a>&#xE9;</a>",
    <opt:options><opt:parse-external-parsed-entity/></opt:options>)))
)
