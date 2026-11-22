jsoniq version "1.0";
(: Keys with digits, separators, quotes, braces, no character at all or non-ASCII
 : characters are kept apart from the others. :)
declare variable $text := "{\"12:{}\":{\"*\":1,\"\":2,\"}\":3},\"\":{\"\\\"\":4},\"é\":{\"é\":5},\"12\":6}";
[
  for $v in parse-json($text)
  return ($v."12:{}"."*", $v."12:{}"."", $v."12:{}"."}", $v.""."\"", $v."é"."é")
]
