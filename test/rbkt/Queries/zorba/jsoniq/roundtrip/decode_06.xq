(: The serialized node is a string, already decoded: the encoding its XML
   declaration names does not apply (F&O 3.1, 14.7.1, Notes). The é stays one
   character, 233, instead of being decoded again from its UTF-8 bytes
   (195 169). :)
for $type in ("element()", "document-node()")
return string-to-codepoints(string(jn:decode-from-roundtrip({
  "Q{http://jsoniq.org/roundtrip}type" : $type,
  "Q{http://jsoniq.org/roundtrip}value" :
    "<?xml version='1.0' encoding='ISO-8859-1'?><a>&#xE9;</a>"
})))
