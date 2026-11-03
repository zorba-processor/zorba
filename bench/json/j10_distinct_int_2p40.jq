(: j10: 20000 consecutive xs:integer above 2^40, where xs:float rounding joins 131072 :)
declare variable $file as xs:string external;

count(distinct-values(for $i in 1 to 20000 return 1099511627776 + $i))
