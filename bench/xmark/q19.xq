(: XMark Q19: items sorted by location :)
declare variable $auction as document-node() external;

for $i in $auction/site/regions//item
let $name := $i/name/text()
order by zero-or-one($i/location) ascending empty greatest
return <item name="{$name}">{$i/location/text()}</item>
