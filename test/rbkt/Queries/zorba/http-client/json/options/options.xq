import module namespace http = "http://zorba.io/modules/http-client";

let $uri := "http://$HTTP_TEST_SERVER/http-test-data/http1.xml"
for $method in http:options( $uri )
order by $method
return $method
