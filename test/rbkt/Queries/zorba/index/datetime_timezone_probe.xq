(: Point probes of value and general hash indexes on date/time keys with and
   without a timezone, under the implicit timezone -06:00 set in the .spec
   file: each probe returns the nodes that eq returns without an index. :)
import module namespace m = "http://www.example.com/datetime-timezone-probe" at
                             "datetime_timezone_probe.xqlib";

m:init();

fn:string-join((
  m:probe("dt", xs:dateTimeStamp("2013-01-13T06:01:53-06:00")),
  m:probe("dt", xs:dateTime("2013-01-13T12:01:53Z")),
  m:probe("dt", xs:dateTime("2013-01-13T06:01:53")),
  m:probe("d", xs:date("2013-01-13-06:00")),
  m:probe("d", xs:date("2013-01-13")),
  m:probe("t", xs:time("12:01:53Z")),
  m:probe("t", xs:time("06:01:53")),
  m:probe("t", xs:time("06:01:53-06:00"))), "&#10;")
