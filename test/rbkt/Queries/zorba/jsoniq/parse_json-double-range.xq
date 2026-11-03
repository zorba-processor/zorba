(: JSONiq: a number with an exponent is an xs:double and "number literals are
 : mapped to atomic values according to the XML Schema specification". XSD 1.1
 : floatingPointRound maps a value beyond the range of xs:double to INF or -INF
 : and rounds one too small to zero: no error, as for xs:double("1e999"). :)
(
  for $m in jn:members(jn:parse-json('[1e999, -1e999, 1E+999, 1e-999]'))
  return (string($m), $m instance of xs:double),
  jn:members(jn:parse-json('[1e999]')) eq xs:double("1e999")
)
