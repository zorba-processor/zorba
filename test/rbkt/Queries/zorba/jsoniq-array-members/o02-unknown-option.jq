jsoniq version "1.0";
(: An unknown option is ignored, next to this one. :)
[ parse-json(concat("[1", "]"), { "zorba-streaming-members" : true, "other" : 2 })[] ]
