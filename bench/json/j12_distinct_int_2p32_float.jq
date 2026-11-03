(: j12: j9 with one xs:float key: this container hashes on xs:float rounding :)
declare variable $file as xs:string external;

count(distinct-values((for $i in 1 to 1000000 return 4294967296 + $i, xs:float(4294967296))))
