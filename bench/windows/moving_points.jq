jsoniq version "1.0";
(: Moving average over 10 points (sliding window). :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
sum(
  for sliding window $w in b:source($source)
    start at $s when true
    only end at $e when $e - $s eq 9
  return avg(b:numbers($w)))
