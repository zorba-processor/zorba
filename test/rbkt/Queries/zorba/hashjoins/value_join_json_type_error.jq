(: A JSON string eq a JSON number raises XPTY0004, with or without an index. :)
declare variable $data := {
  "orders": [ {"oid": 1, "cust": 1}, {"oid": 2, "cust": "1"} ],
  "customers": [ {"id": 1}, {"id": 2} ]
};

string-join(
  for $o in $data.orders[]
  for $c in $data.customers[]
  where $o.cust eq $c.id
  return string($o.oid),
  " ")
