jsoniq version "1.0";
(: Sessions split by a gap above 5 minutes (tumbling window). :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
sum(
  for tumbling window $w in b:source($source)
    start $s previous $p when empty($p) or b:t($s) - b:t($p) gt xs:dayTimeDuration("PT5M")
  return count($w))
