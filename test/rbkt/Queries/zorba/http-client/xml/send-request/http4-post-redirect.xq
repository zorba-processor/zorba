import module namespace http="http://www.zorba-xquery.com/modules/http-client";

declare namespace h = "http://expath.org/ns/http-client";

http:send-request(<h:request href="http://$HTTP_TEST_SERVER/remotequeue" method="POST" follow-redirect="true"/>, (), ());
