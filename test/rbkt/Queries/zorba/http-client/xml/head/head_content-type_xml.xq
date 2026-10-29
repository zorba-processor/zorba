import module namespace http="http://www.zorba-xquery.com/modules/http-client";

declare namespace exp="http://expath.org/ns/http-client";

data(
  http:head("http://$HTTP_TEST_SERVER/cgi-bin/test-xml")
  /exp:header[@name="Content-Type"]/@value
)


