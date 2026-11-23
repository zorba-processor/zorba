(: 100,000 strings, read one by one through the Item API (mtbench --items). :)
for $i in 1 to 100000
return concat("item ", $i)
