jsoniq version "1.0";
(: A condition that calls a function with a window clause of its own. :)
declare function local:pairs($xs as item*) as item*
{
  for sliding window $w in $xs
    start at $s when true
    only end at $e when $e - $s eq 1
  return [ $w ]
};
[
  for tumbling window $w in (1, 2, 3)
    start $s when exists(local:pairs(1 to $s))
  return [ $w ]
]
