jsoniq version "1.0";
(: The value of the option in a variable: it reads the array as a whole, with the same result. :)
declare variable $b external := true;
[ parse-json(concat("[1", "]"), { "zorba-streaming-members" : $b })[] ]
