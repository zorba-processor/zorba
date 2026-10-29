import module namespace http="http://www.zorba-xquery.com/modules/http-client";

http:get-node("http://$HTTP_TEST_SERVER/cgi-bin/test-xml")[2]

