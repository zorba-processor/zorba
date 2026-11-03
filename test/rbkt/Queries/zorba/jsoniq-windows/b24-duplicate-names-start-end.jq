jsoniq version "1.0";
[ for tumbling window $w in (1, 2) start $a when true end $a when true return 1 ]
