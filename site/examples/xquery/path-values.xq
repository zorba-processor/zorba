let $books := doc("../data/books.xml")//book
return (
  <total>{ sum($books/price/xs:decimal(.)) }</total>,
  <latest>{ max($books/@year/xs:integer(.)) }</latest>,
  <authors>{ string-join(distinct-values($books/author), "; ") }</authors>,
  <shouting>{ string-join($books/title ! upper-case(.), " | ") }</shouting>
)
