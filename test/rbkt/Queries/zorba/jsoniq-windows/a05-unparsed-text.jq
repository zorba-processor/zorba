jsoniq version "1.0";
(: Input: a JSON file read with fn:unparsed-text. :)
declare variable $rbktPath as string external;
[
  for tumbling window $w in
      parse-json(unparsed-text($rbktPath || "/Queries/zorba/jsoniq-windows/data/series.json"))[]
    start at $p when $p mod 3 eq 1
  return sum($w.v)
]
