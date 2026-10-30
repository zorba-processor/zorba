(: Value join on JSON numbers (static type xs:anyAtomicType?): integers,
   decimals and doubles that are equal match each other; -1 must not match 1
   (normalized keys collide on the absolute value); 0 matches -0e0; missing
   keys match nothing. :)
declare variable $data := {
  "orders": [
    {"oid": 1, "cust": 1}, {"oid": 2, "cust": 1.0}, {"oid": 3, "cust": 1e0},
    {"oid": 4, "cust": -1}, {"oid": 5, "cust": 2}, {"oid": 6, "cust": 0},
    {"oid": 7, "cust": -0e0}, {"oid": 8}, {"oid": 9, "cust": 3.5},
    {"oid": 10, "cust": 7}
  ],
  "customers": [
    {"id": 1, "name": "a"}, {"id": 2.0, "name": "b"}, {"id": 0, "name": "z"},
    {"id": -2, "name": "m"}, {"id": 3.5e0, "name": "c"}, {"name": "e"},
    {"id": -1.0, "name": "n"}, {"id": 1e0, "name": "d"}
  ]
};

string-join(
  for $o in $data.orders[]
  for $c in $data.customers[]
  where $o.cust eq $c.id
  order by $o.oid, $c.name
  return $o.oid || $c.name,
  " ")
