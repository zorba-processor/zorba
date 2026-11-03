import module namespace http = "http://zorba.io/modules/http-client";
import module namespace file = "http://expath.org/ns/file";

declare variable $rbktPath as xs:string external;

(: What the server received, before any XML parsing of its answer. :)
variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",
  "body": {
    "media-type": "text/plain",
    "content": file:read-text(concat($rbktPath, "/Queries/zorba/bom/data/bom.txt"))
  }
};

string-to-codepoints(substring-after(
  string(http:send-request($req)("body")("content")), "<![CDATA["))[1]
