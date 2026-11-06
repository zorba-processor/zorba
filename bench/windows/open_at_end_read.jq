jsoniq version "1.0";
(: As open_at_end, with every window read: the output has n(n+1)/2 items. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
sum(
  for sliding window $w in b:source($source)
    start when true
    end $e when $e.id lt 0
  return count($w))
