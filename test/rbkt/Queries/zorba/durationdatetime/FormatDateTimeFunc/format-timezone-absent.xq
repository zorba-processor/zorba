(: F&O 3.0 9.8.4.1: a timezone component is ignored when the value has no
   timezone; the military format uses J for that case. :)
string-join((
  format-date(xs:date("2000-02-29"), "[Z]"),
  format-date(xs:date("2000-02-29"), "[z]"),
  format-date(xs:date("2000-02-29"), "[ZN]"),
  format-date(xs:date("2000-02-29"), "[Z0000]"),
  format-date(xs:date("2000-02-29"), "[ZZ]"),
  format-dateTime(xs:dateTime("2000-01-01T08:30:00"), "[H01]:[m01][Z]"),
  format-time(xs:time("08:30:00"), "[H01]:[m01][ZN]", (), (), "America/New_York")
), "|")
