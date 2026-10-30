let $lib := doc("../data/books.xml")
return (
  $lib//tag[. = "science fiction"]/ancestor::book/@id/string(),
  $lib//title[. = "Neuromancer"]/following-sibling::author/string(),
  $lib//book[@id = "b3"]/preceding-sibling::book/@id/string(),
  count($lib//book[1]/descendant::*),
  $lib//price[. > 20]/../@year/string()
)
