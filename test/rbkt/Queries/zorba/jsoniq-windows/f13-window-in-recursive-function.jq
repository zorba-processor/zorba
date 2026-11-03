jsoniq version "1.0";
(: A window inside a recursive function: pairwise sums until one value is left.
   (1,2,3,4,5) -> (3,7,5) -> (10,5) -> 15 :)
declare function local:rollup($xs as item*) as item*
{
  if (count($xs) le 1)
  then $xs
  else local:rollup(
    for tumbling window $w in $xs
      start at $s when true
      end at $e when $e - $s eq 1
    return sum($w))
};
[ local:rollup((1, 2, 3, 4, 5)) ]
