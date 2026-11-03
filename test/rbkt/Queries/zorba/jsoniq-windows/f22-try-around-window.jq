jsoniq version "1.0";
(: An error raised by a condition reaches an enclosing try. :)
[
  try {
    [ for tumbling window $w in (1, 2) start $s when 1 div ($s - 2) gt 0 return $s ]
  } catch * {
    "caught"
  }
]
