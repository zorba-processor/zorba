jsoniq version "1.0";
(: A group by after a window clause with no end clause or with conditions that
   bind no variables, sliding and tumbling. :)
let $a := [ {"cat":"a"}, {"cat":"b"}, {"cat":"a"} ]
return [
  [ for tumbling window $w in $a[] start when true group by $k := $w[1].cat order by $k return [ $k, count($w) ] ],
  [ for tumbling window $w in $a[] start when true end when true group by $k := $w[1].cat order by $k return [ $k, count($w) ] ],
  [ for sliding window $w in $a[] start when true end when true group by $k := $w[1].cat order by $k return [ $k, count($w) ] ],
  [ for tumbling window $w in $a[] start $s when true group by $k := $s.cat order by $k return [ $k, count($w) ] ]
]
