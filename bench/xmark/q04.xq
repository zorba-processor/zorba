(: XMark Q4: reserves of auctions where person20 bid before person51 :)
declare variable $auction as document-node() external;

for $a in $auction/site/open_auctions/open_auction
where some $x in $a/bidder/personref[@person = "person20"],
           $y in $a/bidder/personref[@person = "person51"]
      satisfies $x << $y
return <history>{$a/reserve/text()}</history>
