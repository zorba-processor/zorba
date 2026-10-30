(: Value join on JSON strings (static type xs:anyAtomicType?), grouped. :)
declare variable $data := {
  "orders": [
    {"oid": 1, "cust": "c1", "total": 10}, {"oid": 2, "cust": "c2", "total": 5},
    {"oid": 3, "cust": "c1", "total": 7}, {"oid": 4, "cust": "C1", "total": 1},
    {"oid": 5, "cust": "c9", "total": 2}, {"oid": 6, "total": 4},
    {"oid": 7, "cust": "", "total": 3}
  ],
  "customers": [
    {"id": "c1", "country": "MX"}, {"id": "c2", "country": "DE"},
    {"id": "c3", "country": "MX"}, {"id": "", "country": "XX"}
  ]
};

string-join(
  for $o in $data.orders[]
  for $c in $data.customers[]
  where $o.cust eq $c.id
  group by $country := $c.country
  order by $country
  return $country || ":" || count($o) || ":" || sum($o.total),
  " ")
