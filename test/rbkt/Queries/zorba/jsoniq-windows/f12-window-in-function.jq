jsoniq version "1.0";
(: A window inside a user function, called twice. :)
declare function local:moving($xs as item*, $n as integer) as item*
{
  for sliding window $w in $xs
    start at $s when true
    only end at $e when $e - $s eq $n - 1
  return avg($w)
};
[ local:moving((1, 2, 3, 4), 2), local:moving((10, 20, 30), 3) ]
