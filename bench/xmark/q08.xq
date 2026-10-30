(: XMark Q8: number of items bought per person (join) :)
declare variable $auction as document-node() external;

for $p in $auction/site/people/person
let $bought := for $c in $auction/site/closed_auctions/closed_auction
               where $c/buyer/@person = $p/@id
               return $c
return <item person="{$p/name/text()}">{count($bought)}</item>
