This directory contains the files served by the HTTP test server used by
the http-client tests (and a few schema/URI-resolution tests).

Tests refer to the server as $HTTP_TEST_SERVER (host:port). The test
drivers replace it with the ZORBA_HTTP_TEST_SERVER environment variable,
or with the ZORBA_HTTP_TEST_SERVER CMake setting (default localhost:8080).

The server needs:
  - docroot/ as the document root, with PHP enabled for *.php;
  - cgi-bin/ mapped to /cgi-bin/ and executable as CGI (Bourne shell);
  - HTTP basic authentication on /http-test-data/basic-auth for user
    "zorba" with password "blub";
  - TRACE enabled.

docker/http-test-server builds such a server (Apache 2.4 + PHP 8.3):

  docker build -t zorba-http-test -f docker/http-test-server/Dockerfile .
  docker run -d -p 8080:8080 zorba-http-test
  ctest -R http-client

docker/test-with-http-server.sh runs the whole suite in a builder image
against that server.
