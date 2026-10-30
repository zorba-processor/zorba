(: The inner FLWOR is unnested, so the join predicate refers to $c, a FOR
   var over a single item that is bound after the loop var $i. The index
   join rule must not build an index whose key refers to $c. :)
for $o in (for $i in 1 to 200 return { "customer": $i mod 50, "total": $i })
for $c in (for $i in 0 to 49 return { "id": $i, "country": "C" || ($i mod 7) })
where xs:integer($o.customer) eq xs:integer($c.id)
group by $country := $c.country
order by $country
return $country || ":" || count($o) || ":" || sum($o.total)
