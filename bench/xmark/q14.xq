(: XMark Q14: items whose description contains gold (full-text scan) :)
declare variable $auction as document-node() external;

for $i in $auction/site//item
where contains(string(exactly-one($i/description)), "gold")
return $i/name/text()
