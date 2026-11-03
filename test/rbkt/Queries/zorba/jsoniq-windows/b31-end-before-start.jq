jsoniq version "1.0";
(: XQuery 3.0 grammar [51]: the start condition comes first. :)
[ for tumbling window $w in (1, 2) end when true start when true return 1 ]
