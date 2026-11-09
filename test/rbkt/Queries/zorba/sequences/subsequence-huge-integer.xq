(: fn:subsequence and positional predicates with integer arguments at the
   limits of xs:long. An error is reported as its code. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:t($label as xs:string, $f as function() as item()*)
  as xs:string
{
  try
  {
    $label || ": " || string-join(for $i in $f() return string($i), ",")
  }
  catch *
  {
    $label || ": " || $err:code
  }
};

let $s := (1, 2, 3)
let $max := xs:integer("9223372036854775807")
let $min := xs:integer("-9223372036854775808")
let $min1 := xs:integer("-9223372036854775807")
return
string-join((
  local:t("start max", function() { subsequence($s, $max) }),
  local:t("start min", function() { subsequence($s, $min) }),
  local:t("min, max", function() { subsequence($s, $min, $max) }),
  local:t("min1, max", function() { subsequence($s, $min1, $max) }),
  local:t("1, max", function() { subsequence($s, 1, $max) }),
  local:t("2, max", function() { subsequence($s, 2, $max) }),
  local:t("0, max", function() { subsequence($s, 0, $max) }),
  local:t("max, min", function() { subsequence($s, $max, $min) }),
  local:t("min, min", function() { subsequence($s, $min, $min) }),
  local:t("2, min", function() { subsequence($s, 2, $min) }),
  local:t("position ge min", function() { $s[position() ge $min] }),
  local:t("position gt max", function() { $s[position() gt $max] }),
  local:t("position lt max", function() { $s[position() lt $max] }),
  local:t("position le max", function() { $s[position() le $max] }),
  local:t("position lt min", function() { $s[position() lt $min] }),
  local:t("position eq max", function() { $s[position() eq $max] }),
  local:t("position eq min", function() { $s[position() eq $min] })
), codepoints-to-string(10))
