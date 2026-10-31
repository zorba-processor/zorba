import module namespace http = "http://zorba.io/modules/http-client";

(: A status line without a reason phrase ("HTTP/1.1 404 "; HTTP/2 never has
   one) and a header with an empty value used to abort the http-client
   module on an out-of-range string index (QT3 fn-unparsed-text-019). :)
let $res := http:send-request({
  "href": "http://$HTTP_TEST_SERVER/cgi-bin/nph-empty-reason",
  "method": "GET"
})
return (
  $res("status"),
  $res("message") eq "",
  $res("headers")("X-Empty") eq ""
)
