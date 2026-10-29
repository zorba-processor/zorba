import module namespace http="http://www.zorba-xquery.com/modules/http-client";

http:get("http://$HTTP_TEST_SERVER/http-test-data/search.json")[2]