(: XMark Q13: Australian items with descriptions :)
declare variable $auction as document-node() external;

for $i in $auction/site/regions/australia/item
return <item name="{$i/name/text()}">{$i/description}</item>
