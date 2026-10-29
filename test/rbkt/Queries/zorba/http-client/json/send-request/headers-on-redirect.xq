jsoniq version "1.0";
import module namespace http="http://zorba.io/modules/http-client";

http:send-request(
{
  "href": "http://$HTTP_TEST_SERVER/remotequeue",
  "method": "HEAD",
  "options": {
    "follow-redirect": true
  }  
}).headers("Content-Type")