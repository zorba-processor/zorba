for $x in (xs:date("2010-10-10"))
for $y in (adjust-date-to-timezone(xs:date("2010-10-10"), implicit-timezone()), xs:date("2010-10-10+03:00"))
where $x = $y
return $x