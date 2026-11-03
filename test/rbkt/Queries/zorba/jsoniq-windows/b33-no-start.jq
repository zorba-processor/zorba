jsoniq version "1.0";
[ for tumbling window $w in (1, 2) end when true return 1 ]
