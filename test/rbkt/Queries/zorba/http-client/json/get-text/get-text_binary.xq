jsoniq version "1.0";
import module namespace http="http://zorba.io/modules/http-client";

http:get-text("http://$HTTP_TEST_SERVER/cgi-bin/test-binary").body.content