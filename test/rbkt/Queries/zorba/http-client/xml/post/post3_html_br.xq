import module namespace http="http://www.zorba-xquery.com/modules/http-client";

(: Should get serialized as "<br>" with HTML output method :)
variable $arg := <br></br>;
variable $result :=
  http:post("http://$HTTP_TEST_SERVER/cgi-bin/test-text", $arg,
           "text/html");

$result[2]

