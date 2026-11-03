(: j14: j10 with an xs:float key first :)
declare variable $file as xs:string external;

count(distinct-values((xs:float(1099511627776), for $i in 1 to 20000 return 1099511627776 + $i)))
