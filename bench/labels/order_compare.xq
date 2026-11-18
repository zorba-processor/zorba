(: Document order comparisons between nodes of one tree, labels compared
   directly. :)
let $doc := parse-xml(string-join(("<r>",
  for $i in 1 to 20000 return "<a><b/><c><d/></c></a>", "</r>")))
let $bs := $doc//b
let $ds := $doc//d
let $n := count($bs)
return count(
  for $k in 0 to 49
  for $i in 1 to $n
  where $bs[$i] << $ds[($i * 7 + $k) mod $n + 1]
  return 1)
