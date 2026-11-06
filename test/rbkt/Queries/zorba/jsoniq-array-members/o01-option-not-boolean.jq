jsoniq version "1.0";
(: An option that is not a boolean raises, as the other options do. :)
parse-json(concat("[1", "]"), { "zorba-streaming-members" : 1 })[]
