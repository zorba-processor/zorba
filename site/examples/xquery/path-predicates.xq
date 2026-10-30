let $lib := doc("../data/books.xml")
return (
  $lib//book[price < 20][@lang = "en"]/title/string(),
  $lib//book[2]/@id/string(),
  ($lib//book)[last()]/title/string(),
  $lib//book[tags/tag = "essay"]/author/string()
)
