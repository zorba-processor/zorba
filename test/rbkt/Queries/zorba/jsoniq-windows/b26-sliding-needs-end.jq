jsoniq version "1.0";
(: XQuery 3.0 grammar [52]: the end condition of a sliding window is mandatory. :)
[ for sliding window $w in (1, 2) start when true return 1 ]
