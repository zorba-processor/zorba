jsoniq version "1.0";
(: Input: dir:documents over JSON, JSON Lines and CSV files, a window per file. :)
import module namespace dir = "http://zorba.io/modules/directory";
declare variable $rbktPath as string external;
[
  for $d in dir:documents($rbktPath || "/Queries/zorba/jsoniq-windows/data/tree")
  order by $d.path
  for sliding window $w in $d.content[]
    start at $s when true
    only end at $e when $e - $s eq 1
  return { "path": $d.path, "sum": sum($w.v) }
]
