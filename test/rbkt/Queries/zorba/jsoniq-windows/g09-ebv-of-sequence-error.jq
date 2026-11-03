jsoniq version "1.0";
(: F&O 3.0 7.3.1: no effective boolean value for several atomic items. :)
let $a := [ {"xs":[1,2]} ]
return [ for tumbling window $w in $a[] start $s when $s.xs[] return 1 ]
