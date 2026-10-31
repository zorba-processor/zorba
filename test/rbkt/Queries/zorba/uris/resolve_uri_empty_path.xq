(: Merging with a base URI that has an authority and an empty path prefixes
   "/" (RFC 3986 5.2.3; F&O 3.0 6.1). :)
(resolve-uri("a", "http://a"),
 resolve-uri("a/b?q", "http://example.org"),
 resolve-uri("a", "http://a/b/c"))
