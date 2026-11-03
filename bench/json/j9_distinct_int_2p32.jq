(: j9: 1000000 consecutive xs:integer above 2^32, where xs:float rounding joins 512 :)
declare variable $file as xs:string external;

count(distinct-values(for $i in 1 to 1000000 return 4294967296 + $i))
