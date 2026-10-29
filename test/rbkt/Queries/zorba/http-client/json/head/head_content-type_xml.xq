jsoniq version "1.0";
import module namespace http="http://zorba.io/modules/http-client";

declare namespace exp="http://expath.org/ns/http-client";

http:head("http://$HTTP_TEST_SERVER/cgi-bin/test-xml").headers("Content-Type")