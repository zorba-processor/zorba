(: An empty $href: unparsed-text-available() returns false and
   unparsed-text()/unparsed-text-lines() the empty sequence. The first used
   to dereference a null item (QT3 fn-unparsed-text-available-053/-054). :)
let $none := "x"[current-date() lt xs:date("1900-01-01")]
return (
  fn:unparsed-text-available($none),
  fn:unparsed-text-available($none, "utf-8"),
  count(fn:unparsed-text($none)),
  count(fn:unparsed-text-lines($none, "utf-8"))
)
