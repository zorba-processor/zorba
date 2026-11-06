jsoniq version "1.0";
(: Without only end, all the windows are returned when the input ends; none is read. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(
  for sliding window $w in b:source($source)
    start when true
    end $e when $e.id lt 0
  return 1)
