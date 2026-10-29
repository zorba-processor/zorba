import module namespace http="http://www.zorba-xquery.com/modules/http-client";

variable $arg := <hello>there</hello>;
variable $result :=
  http:post("http://$HTTP_TEST_SERVER/cgi-bin/test-text", $arg,
           "text/plain");

$result[2]

