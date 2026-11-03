jsoniq version "1.0";
(: JSONiq 5.5.1: any NCName is an unquoted key, the clause keywords among them. :)
let $a := [ {"start":1,"end":2,"when":3,"only":4,"previous":5,"next":6,"at":7,"window":8,"tumbling":9,"sliding":10} ]
return [
  for tumbling window $w in $a[]
    start $s when $s.start eq 1
    end $e when $e.end eq 2
  return [ $s.when, $s.only, $s.previous, $s.next, $s.at, $s.window, $s.tumbling, $s.sliding ]
]
