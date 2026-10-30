(: Value join on untyped attributes (static type xs:anyAtomicType?): the
   index join rule uses normalized keys and keeps the predicate. :)
let $auction := doc("auction.xml")
for $p in $auction/site/people/person
let $a :=
  for $t in $auction/site/closed_auctions/closed_auction
  where $t/buyer/@person eq $p/@id
  return $t
return <item person="{$p/name/text()}">{count($a)}</item>
