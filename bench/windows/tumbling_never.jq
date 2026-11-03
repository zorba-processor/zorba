jsoniq version "1.0";
(: One tumbling window that never closes, with its variable read. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(
  for tumbling window $w in b:source($source)
    start when true
    only end $e when $e.id lt 0
  return count($w))
