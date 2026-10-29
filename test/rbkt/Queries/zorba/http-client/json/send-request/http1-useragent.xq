import module namespace http="http://zorba.io/modules/http-client";

variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",  
  "body": {
    "media-type": "text/plain",
    "content": "
      Dies ist ein kleiner Test
    "
  },
  "options": {
    "user-agent": "zorba-test"
  }
};

parse-xml(http:send-request($req)("body")("content"))//*:header[@*:name="User-Agent"]/@value/string(.)