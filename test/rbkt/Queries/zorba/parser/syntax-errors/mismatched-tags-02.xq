(: XQuery 3.0 3.9.1: XQST0118 for an end tag that does not match the start
   tag, also with attributes and with only the prefix differing. :)
declare namespace a = "urn:x";
declare namespace b = "urn:x";
<a:e x="1"></b:e>
