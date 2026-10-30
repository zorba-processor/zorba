(: XMark Q1: name of person0 :)
declare variable $auction as document-node() external;

for $p in $auction/site/people/person[@id = "person0"] return $p/name/text()
