import module namespace http="http://www.zorba-xquery.com/modules/http-client";

http:get-binary("http://$HTTP_TEST_SERVER/cgi-bin/test-xml?querystring")[2]

