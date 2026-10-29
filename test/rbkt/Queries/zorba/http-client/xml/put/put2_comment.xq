import module namespace http="http://www.zorba-xquery.com/modules/http-client";

variable $arg := <!-- comment -->;
variable $result :=
  http:put("http://$HTTP_TEST_SERVER/cgi-bin/test-text", $arg);

$result[2]

