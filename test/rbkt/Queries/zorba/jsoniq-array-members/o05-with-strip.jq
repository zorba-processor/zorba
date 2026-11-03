jsoniq version "1.0";
(: With the top-level array stripped, the option has no effect. :)
[ parse-json(concat("[[1,2],", "[3]]"), { "zorba-streaming-members" : true, "jsoniq-strip-top-level-array" : true })[] ]
