jsoniq version "1.0";
(: XQuery 3.0 3.10.4: end variables are not in scope in the start condition. :)
[ for tumbling window $w in (1, 2) start $s when $e gt 0 end $e when true return 1 ]
