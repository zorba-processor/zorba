import module namespace http="http://www.zorba-xquery.com/modules/http-client";

variable $result :=
  http:post("http://$HTTP_TEST_SERVER/cgi-bin/test-text", 5);

$result[2]

