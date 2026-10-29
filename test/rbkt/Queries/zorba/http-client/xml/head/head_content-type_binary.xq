import module namespace http="http://www.zorba-xquery.com/modules/http-client";

declare namespace exp="http://expath.org/ns/http-client";

data(
  http:head("http://$HTTP_TEST_SERVER/cgi-bin/test-binary")
  /exp:header[@name="Content-Type"]/@value
)


