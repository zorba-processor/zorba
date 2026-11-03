(: j13: j9 with an xs:float key first: every key hashes on its xs:float rounding from the start :)
declare variable $file as xs:string external;

count(distinct-values((xs:float(4294967296), for $i in 1 to 1000000 return 4294967296 + $i)))
