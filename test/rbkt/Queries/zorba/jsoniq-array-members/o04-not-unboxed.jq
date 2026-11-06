jsoniq version "1.0";
(: Without unboxing, the option has no effect. :)
size(parse-json(concat("[1,", "2]"), { "zorba-streaming-members" : true }))
