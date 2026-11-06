import module namespace http = "http://zorba.io/modules/http-client";
import module namespace fetch = "http://zorba.io/modules/fetch";

(: The string of an HTTP response is a stream that cannot seek. :)
variable $u := "http://$HTTP_TEST_SERVER/http-test-data/";
jn:members(jn:parse-json(http:get(concat($u, "bom-json.txt"))("body")("content"))),
substring(fetch:content(concat($u, "http1.xml")), 1, 1),
substring(unparsed-text(concat($u, "http1.xml")), 1, 1)
