jsoniq version "1.0";
import module namespace http="http://zorba.io/modules/http-client";

variable $result :=
  http:delete("http://$HTTP_TEST_SERVER/cgi-bin/test-text");

$result.body.content
