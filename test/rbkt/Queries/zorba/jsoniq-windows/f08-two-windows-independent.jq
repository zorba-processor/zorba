jsoniq version "1.0";
(: Two independent window clauses: every window of the second per window of the first. :)
[
  for tumbling window $a in (1, 2)
    start when true
  for sliding window $b in ("a", "b")
    start when true
    end when true
  return [ $a, $b ]
]
