(: XMark Q6: items per region :)
declare variable $auction as document-node() external;

for $r in $auction/site/regions return count($r//item)
