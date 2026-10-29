import module namespace http = "http://www.zorba-xquery.com/modules/http-client";

variable $res := 
http:send-request((), "http://$HTTP_TEST_SERVER/cgi-bin/test-xml?querystring", ());

$res[2]

