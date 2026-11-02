jsoniq version "1.0";
[ for tumbling window $w in (1, 2) start when true start when true return 1 ]
