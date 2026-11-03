jsoniq version "1.0";
(: XQuery 3.0 3.10.4: the window variable is not in scope in the conditions. :)
[ for tumbling window $w in (1, 2) start $s when count($w) gt 0 return 1 ]
