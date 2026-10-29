<http:request xmlns:http="http://expath.org/ns/http-client" method="POST" href="$HTTP_TEST_SERVER/http-test-data/request.php">
  <http:header name="Host" value="$HTTP_TEST_SERVER"/>
  <http:header name="User-Agent" value="libcurl-agent/1.0"/>
  <http:header name="Accept" value="*/*"/>
  <http:header name="foo" value="bar"/>
  <http:header name="Content-Type" value="multipart/form-data; boundary=----------------------------4ebf00fbcf09"/>
  <http:header name="Content-Length" value="37"/>
  <http:multipart content-type="multipart/form-data" boundary="----------------------------4ebf00fbcf09">
  </http:multipart>
</http:request>
