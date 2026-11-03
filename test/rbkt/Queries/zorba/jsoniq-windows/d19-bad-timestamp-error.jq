jsoniq version "1.0";
(: F&O 3.0 19.1: casting a string that is not a dateTime raises FORG0001. :)
let $a := [ {"ts":"2026-01-01T00:00:00Z"}, {"ts":"not a date"} ]
return [ for tumbling window $w in $a[] start $s when xs:dateTime($s.ts) lt xs:dateTime("2030-01-01T00:00:00Z") return 1 ]
