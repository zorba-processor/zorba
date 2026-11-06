import module namespace http = "http://zorba.io/modules/http-client";

(: A U+FEFF at the start of a string body is text: in UTF-8 it reaches the server.
 : Without a charset the body goes in ISO-8859-1, where ICU drops it. :)
variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",
  "body": {
    "media-type": "text/plain; charset=UTF-8",
    "content": "&#xFEFF;x"
  }
};

string-to-codepoints(substring-after(
  string(http:send-request($req)("body")("content")), "<![CDATA["))[1]
