declare variable $s external := "[1]";
declare variable $b external := true;
parse-json($s, { "zorba-streaming-members" : $b })[]
