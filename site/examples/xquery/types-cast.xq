(
  xs:integer("42") + 1,
  "4x2" castable as xs:integer,
  "2026-10-29" cast as xs:date,
  xs:date("2026-10-29") + xs:dayTimeDuration("P3D"),
  xs:date("2026-12-25") - xs:date("2026-10-29"),
  3 instance of xs:decimal,
  3.0 instance of xs:integer,
  1 div 3,
  1e0 div 3
)
