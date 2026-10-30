(: XMark Q5: closed auctions with price >= 40 :)
declare variable $auction as document-node() external;

count(for $c in $auction/site/closed_auctions/closed_auction
      where $c/price/text() >= 40
      return $c/price)
