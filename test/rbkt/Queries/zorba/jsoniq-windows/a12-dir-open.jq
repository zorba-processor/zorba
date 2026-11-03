jsoniq version "1.0";
(: Input: dir:open on one file. :)
import module namespace dir = "http://zorba.io/modules/directory";
declare variable $rbktPath as string external;
[
  for tumbling window $w in dir:open($rbktPath || "/Queries/zorba/jsoniq-windows/data/tree", "a.json").content[]
    start at $s when true
    end at $e when $e - $s eq 1
  return [ $w.v ]
]
