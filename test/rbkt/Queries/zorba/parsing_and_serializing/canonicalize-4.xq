import module namespace x = "http://zorba.io/modules/xml";

(: Canonical XML cannot represent a relative namespace URI. :)
x:canonicalize('<a xmlns:p="relative"><p:b/></a>')
