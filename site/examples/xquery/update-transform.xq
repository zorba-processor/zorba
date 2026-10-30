let $book := doc("../data/books.xml")//book[@id = "b3"]
return
  copy $b := $book
  modify (
    replace value of node $b/price with "9.99",
    insert node <tag>cyberpunk</tag> into $b/tags,
    delete node $b/@lang,
    rename node $b/author as "writer"
  )
  return $b
