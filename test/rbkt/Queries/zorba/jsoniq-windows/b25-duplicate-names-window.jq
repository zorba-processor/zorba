jsoniq version "1.0";
[ for tumbling window $w in (1, 2) start $w when true return 1 ]
