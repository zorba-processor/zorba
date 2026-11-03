import module namespace http = "http://zorba.io/modules/http-client";

(: A U+FEFF at the start of a string body is text: it reaches the server. :)
variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",
  "body": {
    "media-type": "text/plain",
    "content": "&#xFEFF;x"
  }
};

string-to-codepoints(substring-after(
  string(http:send-request($req)("body")("content")), "<![CDATA["))[1]
