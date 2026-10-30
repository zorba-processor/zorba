(: XMark Q15: emphasized keywords in closed auction annotations (long path) :)
declare variable $auction as document-node() external;

for $k in $auction/site/closed_auctions/closed_auction/annotation/description/parlist/listitem/parlist/listitem/text/emph/keyword/text()
return <text>{$k}</text>
