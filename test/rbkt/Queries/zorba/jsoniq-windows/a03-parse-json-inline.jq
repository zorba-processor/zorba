jsoniq version "1.0";
(: Input: objects produced by parse-json. :)
[
  for tumbling window $w in parse-json('[{"t":1,"v":10},{"t":2,"v":20},{"t":3,"v":30}]')[]
    start at $p when $p mod 2 eq 1
  return sum($w.v)
]
