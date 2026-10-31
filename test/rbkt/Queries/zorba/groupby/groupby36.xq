(: The grouping key must be bound to a variable. :)
for $x in 1 to 10
group by $k := $x mod 2
order by $k
return count($x)
