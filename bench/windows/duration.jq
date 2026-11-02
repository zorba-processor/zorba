jsoniq version "1.0";
(: Averages over windows of 1 minute from their first record (tumbling window). :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
sum(
  for tumbling window $w in b:source($source)
    start $s when true
    end next $n when empty($n) or b:t($n) - b:t($s) ge xs:dayTimeDuration("PT1M")
  return avg(b:numbers($w)))
