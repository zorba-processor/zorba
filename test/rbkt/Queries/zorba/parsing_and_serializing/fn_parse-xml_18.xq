(: The libxml2 warning for the relative namespace URI quotes "~x", which used
   to be looked up as a diagnostic dictionary key and fail a ZORBA_ASSERT
   inside the libxml2 error callback (xml_parse fuzzer). :)
fn:parse-xml('<a xmlns="~x">')
