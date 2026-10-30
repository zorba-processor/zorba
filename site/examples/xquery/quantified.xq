let $books := doc("../data/books.xml")//book
return (
  <all-priced>{ every $b in $books satisfies $b/price > 0 }</all-priced>,
  <any-under-13>{ some $b in $books satisfies $b/price < 13 }</any-under-13>,
  <general>{ $books/@lang = "en" }</general>,
  <value>{ try { $books/@lang eq "en" } catch * { "eq needs exactly one item" } }</value>
)
