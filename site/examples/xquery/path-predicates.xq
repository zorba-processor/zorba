let $lib := doc("../data/books.xml")
return (
  <cheap-english>{ $lib//book[price < 20][@lang = "en"]/title/string() }</cheap-english>,
  <second-id>{ $lib//book[2]/@id/string() }</second-id>,
  <last-title>{ ($lib//book)[last()]/title/string() }</last-title>,
  <essayist>{ $lib//book[tags/tag = "essay"]/author/string() }</essayist>
)
