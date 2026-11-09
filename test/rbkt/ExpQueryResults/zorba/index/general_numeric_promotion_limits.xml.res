VRange double 2^63: 1 4
VRange float 2^63: 1 4
VRange untyped xs:long max: 1 4
VRange double -2^63: 2
VRange xs:unsignedLong max: 3
VRange double 2^64: 3
VRange decimal 2^63: 4
VRange double 2^53: 5
VRange double 2^53+2: 6 7
VRange double -0: 8 9
VRange float -0: 8 9
VRange double INF: 10
VRange double -INF: 11
VRange float INF: 10 12
VEquality double 2^63: 1 4
VEquality float 2^63: 1 4
VEquality untyped xs:long max: 1 4
VEquality double -2^63: 2
VEquality xs:unsignedLong max: 3
VEquality double 2^64: 3
VEquality decimal 2^63: 4
VEquality double 2^53: 5
VEquality double 2^53+2: 6 7
VEquality double -0: 8 9
VEquality float -0: 8 9
VEquality double INF: 10
VEquality double -INF: 11
VEquality float INF: 10 12
VRange value double 2^63: 1 4
VRange value float 2^63: 1 4
VRange value double 2^53+2: 6 7
VRange value float -0: 8 9
VRange value float INF: 10 12
VEquality value double 2^63: 1 4
VEquality value float 2^63: 1 4
VEquality value double 2^53+2: 6 7
VEquality value float -0: 8 9
VEquality value float INF: 10 12
range double 2^53 lower: 1 3 4 5 6 7 10 12
range exclusive float -0 upper: 2 11
range float INF lower: 10 12
range double 2^63 lower: 1 3 4 10 12
range value float 2^63 lower: 1 3 4 10 12
range value double 2^53 both: 5 6 7
range value decimal and float bounds: 8 9 13
