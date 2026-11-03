jsoniq version "1.0";
(: As never_closes, with a return clause that reads every window it would return: the variable keeps the items, and no window is returned. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(
  for sliding window $w in b:source($source)
    start when true
    only end $e when $e.id lt 0
  return count($w))
