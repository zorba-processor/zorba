import module namespace x = "http://zorba.io/modules/xml";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: Canonical XML cannot represent a relative namespace URI: the error says so. :)
try { x:canonicalize('<a xmlns:p="relative"><p:b/></a>') }
catch err:FOCZ0001 { contains($err:description, "elative namespace") }
