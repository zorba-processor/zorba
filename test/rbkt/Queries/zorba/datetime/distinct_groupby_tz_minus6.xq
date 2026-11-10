(: fn:distinct-values and group by on date/time values with and without a
   timezone, under the implicit timezone -06:00 (TZ=UTC+6, see
   test/rbkt/Queries/CMakeLists.txt): values that eq finds equal are one
   distinct value and one group. The values are empty under any other implicit
   timezone. :)
let $values :=
  if (implicit-timezone() eq xs:dayTimeDuration("-PT6H"))
  then (xs:dateTime("2013-01-13T06:01:53"),
        xs:dateTimeStamp("2013-01-13T06:01:53-06:00"),
        xs:dateTime("2013-01-13T12:01:53Z"),
        xs:date("2013-01-13"),
        xs:date("2013-01-13-06:00"),
        xs:time("06:01:53"),
        xs:time("12:01:53Z"),
        xs:dateTime("2013-01-13T20:00:00"),
        xs:dateTime("2013-01-14T02:00:00Z"),
        xs:time("06:01:53.5"),
        xs:time("12:01:53.5Z"),
        xs:gYear("2013"),
        xs:gYear("2013-06:00"))
  else ()
return (
  count(distinct-values($values)),
  for $v in $values
  group by $k := $v
  order by count($v) descending
  return count($v)
)
