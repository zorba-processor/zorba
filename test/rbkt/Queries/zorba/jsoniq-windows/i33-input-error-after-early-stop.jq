jsoniq version "1.0";
(: XQuery 3.0 2.3.4: the input is read as it goes, so a consumer that stops before an error in it does not see the error. :)
[ (
  for sliding window $w in (1, 2, 3, error())
    start $s when true
    only end $e when $e - $s eq 1
  return [ $w ])[1] ]
