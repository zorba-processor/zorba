jsoniq version "1.0";
(: The end condition reads the start item: it has its own value per open window. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(
  for sliding window $w in b:source($source)
    start $s when true
    only end $e when $e.id lt $s.id
  return 1)
