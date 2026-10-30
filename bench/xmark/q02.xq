(: XMark Q2: first bid increase of every open auction :)
declare variable $auction as document-node() external;

for $a in $auction/site/open_auctions/open_auction
return <increase>{$a/bidder[1]/increase/text()}</increase>
