jsoniq version "1.0";
(: Averages over consecutive blocks of $k points (tumbling window). :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
declare variable $k as string external;
let $size := integer($k)
return sum(
  for tumbling window $w in b:source($source)
    start at $s when true
    end at $e when $e - $s eq $size - 1
  return avg(b:numbers($w)))
