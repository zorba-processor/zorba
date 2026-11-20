(: Loading: a 100 000-record document parsed from a string; the loader labels
   every node incrementally. :)
let $s := string-join(("<r>",
  for $i in 1 to 100000
  return ("<a i='", string($i), "'><b>", string($i), "</b><c/><d j='x'/></a>"),
  "</r>"))
return count(parse-xml($s)//*)
