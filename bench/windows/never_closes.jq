jsoniq version "1.0";
(: Sliding windows that never close: every record stays a candidate window. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(
  for sliding window $w in b:source($source)
    start when true
    only end $e when $e.id lt 0
  return 1)
