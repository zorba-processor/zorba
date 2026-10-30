(: XMark Q7: count of descriptions, annotations and email addresses :)
declare variable $auction as document-node() external;

for $s in $auction/site
return count($s//description) + count($s//annotation) + count($s//emailaddress)
