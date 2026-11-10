(: xs:dateTimeStamp values probe an index on xs:dateTime keys. :)
let $doc := <d><t>2013-01-13T12:01:53+01:00</t></d>
for $k in (xs:dateTime("2013-01-13T12:01:53+01:00") cast as xs:dateTimeStamp,
           xs:dateTime("2014-01-13T12:01:53+01:00") cast as xs:dateTimeStamp)
let $m := for $t in $doc/t where xs:dateTime($t) = $k return $t
return count($m)
