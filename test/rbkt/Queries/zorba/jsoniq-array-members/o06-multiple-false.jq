jsoniq version "1.0";
(: One value is allowed, a second raises after the members of the first. :)
[ parse-json(concat("[1,2]", " "), { "zorba-streaming-members" : true, "jsoniq-multiple-top-level-items" : false })[] ]
