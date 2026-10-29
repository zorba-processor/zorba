jsoniq version "1.0";
import module namespace http = "http://zorba.io/modules/http-client";

declare default element namespace "http://expath.org/ns/http-client";

variable $req :=
{
  "method": "POST",
  "href": "http://$HTTP_TEST_SERVER/http-test-data/request.php",
  "headers": {"foo":"bar"},
  "body": {
    "media-type": "text/plain",
    "content": "
      Dies ist ein kleiner Test
    "
  }
};

parse-xml(http:send-request($req).body.content)