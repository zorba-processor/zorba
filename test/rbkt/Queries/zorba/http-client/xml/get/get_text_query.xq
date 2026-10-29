import module namespace http="http://www.zorba-xquery.com/modules/http-client";

http:get("http://$HTTP_TEST_SERVER/cgi-bin/test-text?querystring")[2]

