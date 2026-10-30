(: Fractional seconds follow F&O 3.1 9.8.4.5: digits from the picture's
   digit family, truncation, optional digits after mandatory ones, grouping
   separators counted from the left, and width modifiers. :)
string-join((
  format-time(xs:time("09:15:06.456"), "[f๐๐๐]"),
  format-time(xs:time("12:01:01.125"), "[f01]"),
  format-time(xs:time("12:01:01.123"), "[f99#]"),
  format-time(xs:time("12:01:01.12"), "[f99#]"),
  format-time(xs:time("12:01:01.135"), "[f0'0'0]"),
  format-time(xs:time("12:01:01.1"), "[f1###,3-3]"),
  format-time(xs:time("09:15:06.006"), "[f,4-4]"),
  format-time(xs:time("09:15:06.006"), "[f,1-*]"),
  format-time(xs:time("09:15:06.1"), "[f,*-2]"),
  format-time(xs:time("09:15:06.123456"), "[f]")
), "|") eq "๔๕๖|12|123|12|1'3'5|100|0060|006|1|123456"
