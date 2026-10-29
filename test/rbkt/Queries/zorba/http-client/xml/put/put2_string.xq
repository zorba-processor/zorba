import module namespace http="http://www.zorba-xquery.com/modules/http-client";

variable $result :=
  http:put("http://$HTTP_TEST_SERVER/cgi-bin/test-text", "putstring");

$result[2]

