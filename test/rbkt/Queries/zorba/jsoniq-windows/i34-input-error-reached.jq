jsoniq version "1.0";
(: An error in the input surfaces when the clause reaches it. :)
count(
  for sliding window $w in (1, 2, 3, error(xs:QName("FOER0000")))
    start $s when true
    only end $e when $e - $s eq 1
  return [ $w ])
