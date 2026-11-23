(: Orders placed that no shipment names, written as a general = against a LET
   var bound to the distinct ids of the shipments. From its second comparison
   on, the var is hashed: the answer is that of comparing each pair. :)
declare namespace an = "http://zorba.io/annotations";

declare %an:nondeterministic function local:events()
{
  { "kind" : "placed",  "id" : "o1", "shop" : "north" },
  { "kind" : "placed",  "id" : "o2", "shop" : "north" },
  { "kind" : "shipped", "id" : "o1" },
  { "kind" : "placed",  "id" : "o3", "shop" : "south" },
  { "kind" : "placed",  "id" : "o4", "shop" : "north" },
  { "kind" : "shipped", "id" : "o3" },
  { "kind" : "shipped", "id" : "o3" },
  { "kind" : "placed",  "shop" : "south" },
  { "kind" : "placed",  "id" : "o5", "shop" : "south" },
  { "kind" : "placed",  "id" : "o1", "shop" : "south" }
};

let $shipped := distinct-values(local:events()[$$.kind eq "shipped"].id)
for $e in local:events()
where $e.kind eq "placed" and not($e.id = $shipped)
group by $shop := $e.shop
order by $shop
return { "shop" : $shop, "unshipped" : count($e) },

let $shipped := distinct-values(local:events()[$$.kind eq "shipped"].id)
for $e in local:events()
where $e.kind eq "placed" and $shipped = $e.id
return $e.id
