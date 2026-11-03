jsoniq version "1.0";
(: XQuery 3.0 3.10.4: all variables of a window clause must have distinct names. :)
[ for tumbling window $w in (1, 2) start $a at $a when true return 1 ]
