(: JSONiq also requires a grouping variable in each grouping spec. :)
for $x in 1 to 10
group by $x mod 2
return count($x)
