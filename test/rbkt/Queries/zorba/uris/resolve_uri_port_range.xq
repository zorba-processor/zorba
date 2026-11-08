(: A port is accepted only when all of it is digits and it is at most 65535.
   Anything else keeps the authority as written (registry-based authority)
   instead of being truncated or partially parsed. :)
(resolve-uri("x", "http://h:8080/"),
 resolve-uri("x", "http://h:65535/"),
 resolve-uri("x", "http://h:70000/"),
 resolve-uri("x", "http://h:4294967376/"),
 resolve-uri("x", "http://h:80abc/"))
