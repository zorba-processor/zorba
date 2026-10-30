(: XMark Q9: European items bought per person (double join) :)
declare variable $auction as document-node() external;

let $closed := $auction/site/closed_auctions/closed_auction
let $europe := $auction/site/regions/europe/item
for $p in $auction/site/people/person
let $bought := for $c in $closed
               where $p/@id = $c/buyer/@person
               return let $items := for $i in $europe where $c/itemref/@item = $i/@id return $i
                      return <item>{$items/name/text()}</item>
return <person name="{$p/name/text()}">{$bought}</person>
