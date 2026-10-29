import module namespace http="http://www.zorba-xquery.com/modules/http-client";

variable $arg := <h1>hi there</h1>;
variable $result :=
  http:post("http://$HTTP_TEST_SERVER/cgi-bin/test-text", $arg,
           "text/html");

$result[2]

