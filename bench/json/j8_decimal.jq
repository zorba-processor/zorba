(: j8: xs:decimal arithmetic (MAPM): totals with tax, rounding, division :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

let $lines :=
  for $o in $data.orders[], $i in $o.items[]
  return round-half-to-even($i.qty * $i.price * 1.0825, 2)
return (sum($lines), sum($lines) div count($lines), max($lines) - min($lines))
