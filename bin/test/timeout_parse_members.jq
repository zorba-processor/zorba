count(for $i in 1 to 9223372036854775807 return parse-json(concat("[", $i, "]"), { "zorba-streaming-members" : true })[])
