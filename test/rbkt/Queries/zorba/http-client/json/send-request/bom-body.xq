import module namespace http = "http://zorba.io/modules/http-client";
import module namespace file = "http://expath.org/ns/file";

declare variable $rbktPath as xs:string external;

(: The body sent for a string read from a file has no byte order mark. :)
variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",
  "body": {
    "media-type": "text/plain",
    "content": file:read-text(concat($rbktPath, "/Queries/zorba/bom/data/bom.txt"))
  }
};

string-to-codepoints(normalize-space(
  parse-xml(http:send-request($req)("body")("content"))//*:body))[1]
