(: A type that is neither a node kind nor a known type name is the error
   JNTY0023 at the call. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

for $type in ("document()", "Q{urn:x}unknown", "xs:unknown", "", "Q{urn:x",
              "Q{}", "xs:")
return
  try {
    jn:decode-from-roundtrip({
      "Q{http://jsoniq.org/roundtrip}type" : $type,
      "Q{http://jsoniq.org/roundtrip}value" : "<a>x</a>"
    })
  }
  catch * {
    string-join((local-name-from-QName($err:code),
                 $err:line-number, $err:column-number,
                 $err:description) ! string(), " ")
  }
