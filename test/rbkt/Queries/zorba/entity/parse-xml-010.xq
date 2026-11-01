(: W3C QT3 fn-parse-xml/parse-xml-010: the relative system identifier of the
 : external entity resolves against the static base URI. :)
parse-xml("<!DOCTYPE a [<!ELEMENT a (#PCDATA)><!ENTITY foo SYSTEM 'parse-xml/foo.entity'>]><a>" ||
  codepoints-to-string(38) || "foo;</a>")
