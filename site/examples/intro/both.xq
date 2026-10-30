(: One query, two kinds of trees: an XML element and a JSON object. :)
let $book := doc("../data/books.xml")//book[@id = "b3"]
return {
  "title": string($book/title),
  "year": xs:integer($book/@year),
  "tags": [ $book/tags/tag/string() ]
}
