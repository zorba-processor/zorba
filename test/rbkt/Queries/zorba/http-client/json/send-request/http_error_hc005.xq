import module namespace http = "http://zorba.io/modules/http-client";
declare namespace h = "http://expath.org/ns/http-client";

try {
  http:send-request({"hi":"hello"})
} catch http:REQUEST {
  "ok"
},
try {
  http:send-request({"method": 3})
} catch http:REQUEST {
  "ok"
},
try {
  http:send-request({
  "mistypedhref": "http://$HTTP_TEST_SERVER/remotequeue",
  "method": "GET",
  "options": {
    "follow-redirect": false
  }  
})
} catch http:REQUEST {
  "ok"
},
try {
  http:send-request({
  "href": "http://$HTTP_TEST_SERVER/remotequeue",
  "method": 3,
  "options": {
    "follow-redirect": false
  }  
})
} catch http:REQUEST {
  "ok"
},
try {
  http:send-request({
  "href": "http://$HTTP_TEST_SERVER/remotequeue",
  "method": "GET",
  "headers": "three",
  "options": {
    "follow-redirect": false
  }  
})
} catch http:REQUEST {
  "ok"
},
try {
  http:post("http://$HTTP_TEST_SERVER/cgi-bin/test-text", 3,"text/plain")  
} catch http:REQUEST {
  "ok"
},
try {
  http:send-request(
  {
    "method": "POST",
    "href": "www.ggg.ggg",
    "body": {
      "media-type": "text/plain",
      "content": [2,3]
    }
  })
} catch http:REQUEST {
  "ok"
}