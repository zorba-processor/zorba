(: j11: the same as j10 with group by :)
declare variable $file as xs:string external;

count(for $i in 1 to 20000 group by $k := 1099511627776 + $i return $k)
