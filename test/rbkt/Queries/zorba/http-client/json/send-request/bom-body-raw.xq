import module namespace http = "http://zorba.io/modules/http-client";
import module namespace file = "http://expath.org/ns/file";

declare variable $rbktPath as xs:string external;

(: What the server received, in UTF-8: without a charset a text body goes in
 : ISO-8859-1, where ICU drops U+FEFF, and this test could not fail. :)
variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",
  "body": {
    "media-type": "text/plain; charset=UTF-8",
    "content": file:read-text(concat($rbktPath, "/Queries/zorba/bom/data/bom.txt"))
  }
};

string-to-codepoints(substring-after(
  string(http:send-request($req)("body")("content")), "<![CDATA["))[1]
