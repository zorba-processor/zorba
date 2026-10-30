(: XMark Q3: auctions whose last increase is at least twice the first :)
declare variable $auction as document-node() external;

for $a in $auction/site/open_auctions/open_auction
where zero-or-one($a/bidder[1]/increase/text()) * 2 <= $a/bidder[last()]/increase/text()
return <increase first="{$a/bidder[1]/increase/text()}" last="{$a/bidder[last()]/increase/text()}"/>
