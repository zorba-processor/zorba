(: fn:subsequence with huge and non-finite double arguments: positions are
   compared in xs:double arithmetic (F&O 3.1 fn:subsequence). :)
declare function local:t($label as xs:string, $r as item()*) as xs:string
{
  $label || ": " || string-join(for $i in $r return string($i), ",")
};

let $s := (1, 2, 3)
let $inf := xs:double("INF")
let $ninf := xs:double("-INF")
let $nan := xs:double("NaN")
let $big := 1e300
let $nbig := -1e300
return
string-join((
  local:t("start 1e300", subsequence($s, $big)),
  local:t("start -1e300", subsequence($s, $nbig)),
  local:t("start INF", subsequence($s, $inf)),
  local:t("start -INF", subsequence($s, $ninf)),
  local:t("start NaN", subsequence($s, $nan)),
  local:t("1, INF", subsequence($s, 1e0, $inf)),
  local:t("-INF, INF", subsequence($s, $ninf, $inf)),
  local:t("NaN, 2", subsequence($s, $nan, 2e0)),
  local:t("2, NaN", subsequence($s, 2e0, $nan)),
  local:t("1, 1e300", subsequence($s, 1e0, $big)),
  local:t("1, -1e300", subsequence($s, 1e0, $nbig)),
  local:t("-1e300, 1e300", subsequence($s, $nbig, $big)),
  local:t("1e300, -1e300", subsequence($s, $big, $nbig)),
  local:t("2, -INF", subsequence($s, 2e0, $ninf)),
  local:t("-INF, 5", subsequence($s, $ninf, 5e0)),
  local:t("-1e300, 1e300+2", subsequence($s, $nbig, $big + 2e0)),
  local:t("0, 3", subsequence($s, 0e0, 3e0)),
  local:t("2.5, 1.5", subsequence($s, 2.5e0, 1.5e0)),
  local:t("-1, 3.5", subsequence($s, -1e0, 3.5e0)),
  local:t("-1e300, INF", subsequence($s, $nbig, $inf)),
  local:t("INF, 1", subsequence($s, $inf, 1e0))
), codepoints-to-string(10))
