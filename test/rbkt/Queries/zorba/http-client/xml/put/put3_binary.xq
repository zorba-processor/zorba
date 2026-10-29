import module namespace http="http://www.zorba-xquery.com/modules/http-client";

(: value is "hello there" in base64 :)
variable $arg as xs:base64Binary := "aGVsbG8gdGhlcmUK" cast as xs:base64Binary;
variable $result :=
  http:put("http://$HTTP_TEST_SERVER/cgi-bin/test-text", $arg,
           "application/octet-stream");

$result[2]

