jsoniq version "1.0";
(: Evaluating the options raises, next to this option. :)
parse-json(concat("[1", "]"), { "other" : "x" treat as integer, "zorba-streaming-members" : true })[]
