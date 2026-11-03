jsoniq version "1.0";
(: XQuery 3.0 3.10.2: `allowing empty` gives one tuple for an empty binding sequence. :)
let $e := []
return [
  count(for $x allowing empty in $e[] return 1),
  exists(for $x allowing empty in $e[] return 1),
  count(for $x allowing empty in $e[] let $y := 1 return 1)
]
