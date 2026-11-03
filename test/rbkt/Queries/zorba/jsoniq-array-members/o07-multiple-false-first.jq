jsoniq version "1.0";
(: A second value in an option that forbids it, with the member before it. :)
parse-json(concat("[1,2]", "[3"), { "zorba-streaming-members" : true, "jsoniq-multiple-top-level-items" : false })[][1]
