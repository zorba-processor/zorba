(: fn:subsequence and positional predicates with integer arguments at the
   limits of xs:long. :)
declare function local:t($label as xs:string, $r as item()*) as xs:string
{
  $label || ": " || string-join(for $i in $r return string($i), ",")
};

let $s := (1, 2, 3)
let $max := xs:integer("9223372036854775807")
let $min := xs:integer("-9223372036854775808")
let $min1 := xs:integer("-9223372036854775807")
return
string-join((
  local:t("start max", subsequence($s, $max)),
  local:t("start min", subsequence($s, $min)),
  local:t("min, max", subsequence($s, $min, $max)),
  local:t("min1, max", subsequence($s, $min1, $max)),
  local:t("1, max", subsequence($s, 1, $max)),
  local:t("2, max", subsequence($s, 2, $max)),
  local:t("0, max", subsequence($s, 0, $max)),
  local:t("max, min", subsequence($s, $max, $min)),
  local:t("min, min", subsequence($s, $min, $min)),
  local:t("2, min", subsequence($s, 2, $min)),
  local:t("position ge min", $s[position() ge $min]),
  local:t("position gt max", $s[position() gt $max]),
  local:t("position lt max", $s[position() lt $max]),
  local:t("position le max", $s[position() le $max]),
  local:t("position lt min", $s[position() lt $min]),
  local:t("position eq max", $s[position() eq $max]),
  local:t("position eq min", $s[position() eq $min])
), codepoints-to-string(10))
