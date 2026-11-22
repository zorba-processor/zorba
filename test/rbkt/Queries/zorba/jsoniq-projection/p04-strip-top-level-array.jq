jsoniq version "1.0";
(: With jsoniq-strip-top-level-array, what is read is read of each member. :)
declare variable $text := "[{\"a\":{\"b\":1,\"c\":[1]},\"d\":2},{\"a\":{\"b\":2}},[{\"a\":{\"b\":3}}],{\"a\":{\"c\":0}}]";
[
  for $v in parse-json($text, { "jsoniq-strip-top-level-array" : true })
  return $v.a.b
]
