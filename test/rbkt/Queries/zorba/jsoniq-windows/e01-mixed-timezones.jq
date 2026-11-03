jsoniq version "1.0";
(: Strings cast to xs:dateTime compare as instants across time zones (XQuery 3.0
   B.2, op:dateTime-equal). Windows of 30 minutes from their first record. :)
declare function local:t($x) { xs:dateTime($x.ts) };
let $a := [ {"id":1,"ts":"2026-01-01T10:00:00Z"},
            {"id":2,"ts":"2026-01-01T11:30:00+01:00"},
            {"id":3,"ts":"2026-01-01T05:45:00-05:00"},
            {"id":4,"ts":"2026-01-01T11:10:00Z"} ]
return [
  for tumbling window $w in $a[]
    start $s when true
    end next $n when empty($n) or local:t($n) - local:t($s) ge xs:dayTimeDuration("PT30M")
  return {
    "ids": [ $w.id ],
    "start": string(adjust-dateTime-to-timezone(local:t($w[1]), xs:dayTimeDuration("PT0S")))
  }
]
