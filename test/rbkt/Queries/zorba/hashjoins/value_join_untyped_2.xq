(: As value_join_untyped_1, with the join predicate inside a conjunction. :)
let $auction := doc("auction.xml")
for $p in $auction/site/people/person
let $a :=
  for $t in $auction/site/closed_auctions/closed_auction
  where $t/price > 40 and $t/buyer/@person eq $p/@id
  return $t/price/text()
return <item person="{$p/@id}">{$a}</item>
