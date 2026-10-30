(: XMark Q17: persons without a homepage :)
declare variable $auction as document-node() external;

for $p in $auction/site/people/person
where empty($p/homepage/text())
return <person name="{$p/name/text()}"/>
