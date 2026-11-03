import module namespace fetch = "http://zorba.io/modules/fetch";

(: Over HTTP the stream cannot seek: the mark goes when the resource is read. :)
variable $u := "http://$HTTP_TEST_SERVER/http-test-data/";
string-to-codepoints(fetch:content(concat($u, "bom.txt")))[1],
string-to-codepoints(unparsed-text(concat($u, "bom.txt")))[1],
count(unparsed-text-lines(concat($u, "bom.txt"))),
jn:members(jn:parse-json(fetch:content(concat($u, "bom.json"))))
