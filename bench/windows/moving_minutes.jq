jsoniq version "1.0";
(: Moving average over 1 minute from each record (sliding window). :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
sum(
  for sliding window $w in b:source($source)
    start $s when true
    only end next $n when empty($n) or b:t($n) - b:t($s) gt xs:dayTimeDuration("PT1M")
  return avg(b:numbers($w)))
