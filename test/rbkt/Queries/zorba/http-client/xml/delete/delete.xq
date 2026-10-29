import module namespace http="http://www.zorba-xquery.com/modules/http-client";

variable $result :=
  http:delete("http://$HTTP_TEST_SERVER/cgi-bin/test-text");

$result[2]
