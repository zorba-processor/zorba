(: From XML to JSON: build objects and arrays directly from nodes. :)
[
  for $b in doc("../data/books.xml")//book
  where $b/@lang = "en"
  return {
    "id": string($b/@id),
    "title": string($b/title),
    "price": xs:decimal($b/price),
    "tags": [ $b/tags/tag/string() ]
  }
]
