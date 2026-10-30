(: As value_join_unnested_1, with untyped (xs:anyAtomicType?) operands. :)
for $o in (for $i in 1 to 200 return { "customer": $i mod 50, "total": $i })
for $c in (for $i in 0 to 49 return { "id": $i, "country": "C" || ($i mod 7) })
where $o.customer eq $c.id
group by $country := $c.country
order by $country
return $country || ":" || count($o) || ":" || sum($o.total)
