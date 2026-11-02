import module namespace j = "http://jsoniq.org/functions";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: JSONiq: an object key is cast to xs:string, so keys compare as strings: the
   decimal 9223372036854775808.0 gives the key "9223372036854775808" and the
   integer 9223372036854775807 gives "9223372036854775807". :)
(
  for $k in j:keys({ 9223372036854775807 : 1, 9223372036854775808.0 : 2, 9223372036854775807.5 : 3 })
  order by $k
  return $k,
  try { j:keys({ 9223372036854775807 : 1, 9223372036854775807.0 : 2 }) } catch * { local-name-from-QName($err:code) },
  { 9223372036854775807 : "i" }("9223372036854775807"),
  { 9223372036854775808.0 : "d" }("9223372036854775808")
)
