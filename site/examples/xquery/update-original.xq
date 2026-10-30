(: copy/modify works on a copy: the original stays untouched. :)
let $book := doc("../data/books.xml")//book[@id = "b3"]
let $changed :=
  copy $b := $book
  modify replace value of node $b/price with "9.99"
  return $b
return (string($changed/price), string($book/price))
