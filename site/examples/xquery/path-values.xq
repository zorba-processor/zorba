let $books := doc("../data/books.xml")//book
return (
  sum($books/price/xs:decimal(.)),
  max($books/@year/xs:integer(.)),
  distinct-values($books/author),
  string-join($books/title ! upper-case(.), " | ")
)
