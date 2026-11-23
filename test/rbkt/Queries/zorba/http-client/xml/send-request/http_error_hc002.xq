import module namespace http = "http://www.zorba-xquery.com/modules/http-client";
declare namespace h = "http://expath.org/ns/http-client";
declare namespace err = "http://expath.org/ns/error";

http:send-request(<h:request method="GET" href="http://$HTTP_TEST_SERVER/http-test-data/eqs7day-M1.txt" override-media-type="text/xml"/>, (), ())

