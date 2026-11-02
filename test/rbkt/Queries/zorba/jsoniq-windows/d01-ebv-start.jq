jsoniq version "1.0";
(: JSONiq extension 6.1 and 5.2.4: effective boolean value in `when`: missing
   (empty), null, false, 0, "" are false; 1, "x", any object or array are true. :)
let $a := [ {"i":1,"flag":true}, {"i":2}, {"i":3,"flag":false}, {"i":4,"flag":null},
            {"i":5,"flag":1}, {"i":6,"flag":0}, {"i":7,"flag":""}, {"i":8,"flag":"x"},
            {"i":9,"flag":{}}, {"i":10,"flag":[]}, {"i":11,"flag":[false]} ]
return [
  for tumbling window $w in $a[]
    start $s when $s.flag
  return [ $w.i ]
]
