(: Hash joins between date/time values with and without a timezone, under the
   implicit timezone -06:00 (TZ=UTC+6, see test/rbkt/Queries/CMakeLists.txt):
   each join finds the same matches as eq without an index. The data is empty
   under any other implicit timezone. :)
let $doc :=
  if (implicit-timezone() eq xs:dayTimeDuration("-PT6H"))
  then
    <r>
      <e dt="2013-01-13T06:01:53" d="2013-01-13" t="06:01:53"/>
      <e dt="2013-01-13T12:01:53Z" d="2013-01-13-06:00" t="12:01:53Z"/>
      <e dt="2013-01-13T06:01:54" d="2013-01-14" t="06:01:54"/>
    </r>
  else <r/>
return (
  for $x in (xs:dateTimeStamp("2013-01-13T06:01:53-06:00"),
             xs:dateTime("2013-01-13T12:01:53Z"),
             xs:dateTime("2013-01-13T06:01:53"))
  let $m :=
    for $e in $doc/e
    where xs:dateTime($e/@dt) eq $x
    return $e
  return <dateTime v="{$x}">{count($m)}</dateTime>,

  for $x in (xs:date("2013-01-13-06:00"), xs:date("2013-01-13"))
  let $m :=
    for $e in $doc/e
    where xs:date($e/@d) eq $x
    return $e
  return <date v="{$x}">{count($m)}</date>,

  for $x in (xs:time("12:01:53Z"), xs:time("06:01:53"), xs:time("06:01:53-06:00"))
  let $m :=
    for $e in $doc/e
    where xs:time($e/@t) eq $x
    return $e
  return <time v="{$x}">{count($m)}</time>
)
