(: XMark Q18: currency conversion with a user function :)
declare namespace local = "http://www.foobar.org";
declare variable $auction as document-node() external;

declare function local:convert($v as xs:decimal?) as xs:decimal? { 2.20371 * $v };

for $a in $auction/site/open_auctions/open_auction
return local:convert(zero-or-one($a/reserve))
