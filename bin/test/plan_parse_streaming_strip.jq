declare variable $s external := "[1]";
parse-json($s, { "zorba-streaming-members" : true, "jsoniq-strip-top-level-array" : true })[]
