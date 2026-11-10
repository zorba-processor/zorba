(: fn:distinct-values and group by on date/time values with and without a
   timezone, under the implicit timezone -06:00 set in the .spec file: values
   that eq finds equal are one distinct value and one group. :)
let $values := (xs:dateTime("2013-01-13T06:01:53"),
                xs:dateTimeStamp("2013-01-13T06:01:53-06:00"),
                xs:dateTime("2013-01-13T12:01:53Z"),
                xs:date("2013-01-13"),
                xs:date("2013-01-13-06:00"),
                xs:time("06:01:53"),
                xs:time("12:01:53Z"))
return (
  count(distinct-values($values)),
  for $v in $values
  group by $k := $v
  order by count($v) descending
  return count($v)
)
