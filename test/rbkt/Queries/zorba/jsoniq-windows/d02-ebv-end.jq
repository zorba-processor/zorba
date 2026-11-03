jsoniq version "1.0";
(: The same in the end condition. :)
let $a := [ {"i":1}, {"i":2,"flag":null}, {"i":3,"flag":0}, {"i":4,"flag":"y"}, {"i":5} ]
let $b := [ {"i":1}, {"i":2,"flag":null}, {"i":3,"flag":0} ]
return [
  [ for tumbling window $w in $a[] start when true end $e when $e.flag return [ $w.i ] ],
  [ for tumbling window $w in $b[] start when true only end $e when $e.flag return [ $w.i ] ]
]
