(: Path steps whose results are sorted and deduplicated in document order:
   unions, parent and ancestor steps from many context nodes. :)
let $doc := parse-xml(string-join(("<r>",
  for $i in 1 to 20000
  return ("<a><b><c/><c/></b><b><c/></b></a>"),
  "</r>")))
return (
  count($doc//c/..),
  count($doc//c/ancestor::*),
  count(($doc//b | $doc//c | $doc//a)),
  count($doc//c/following-sibling::* union $doc//b/preceding-sibling::*),
  count(reverse($doc//c)/..)
)
