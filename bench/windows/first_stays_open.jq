jsoniq version "1.0";
(: The first window never closes; the others close after 10 points and wait behind it. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(
  for sliding window $w in b:source($source)
    start $s at $p when true
    only end at $q when $p ne 1 and $q - $p eq 9
  return 1)
