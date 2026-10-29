import module namespace http="http://www.zorba-xquery.com/modules/http-client";

http:get-text("http://$HTTP_TEST_SERVER/cgi-bin/test-binary")[2]

