let $lib := doc("../data/books.xml")
return (
  <ancestor>{ $lib//tag[. = "science fiction"]/ancestor::book/@id/string() }</ancestor>,
  <following-sibling>{ $lib//title[. = "Neuromancer"]/following-sibling::author/string() }</following-sibling>,
  <preceding-sibling>{ $lib//book[@id = "b3"]/preceding-sibling::book/@id/string() }</preceding-sibling>,
  <descendants>{ count($lib//book[1]/descendant::*) }</descendants>,
  <parent>{ $lib//price[. > 20]/../@year/string() }</parent>
)
