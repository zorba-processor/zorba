(: XMark Q16: sellers of auctions with emphasized keywords :)
declare variable $auction as document-node() external;

for $c in $auction/site/closed_auctions/closed_auction
where not(empty($c/annotation/description/parlist/listitem/parlist/listitem/text/emph/keyword/text()))
return <person id="{$c/seller/@person}"/>
