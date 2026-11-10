(: xs:dateTime values probe an index on xs:dateTimeStamp keys. :)
let $doc := <d><t>2013-01-13T12:01:53+01:00</t></d>
for $k in (xs:dateTime("2013-01-13T12:01:53+01:00"),
           xs:dateTime("2014-01-13T12:01:53+01:00"))
let $m := for $t in $doc/t
          where (xs:dateTime($t) cast as xs:dateTimeStamp) = $k
          return $t
return count($m)
