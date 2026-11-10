(: Value join on untyped attributes (static type xs:anyAtomicType?): eq can
   raise an error for such operands, so the join runs without an index. :)
let $auction := doc("auction.xml")
for $p in $auction/site/people/person
let $a :=
  for $t in $auction/site/closed_auctions/closed_auction
  where $t/buyer/@person eq $p/@id
  return $t
return <item person="{$p/name/text()}">{count($a)}</item>
