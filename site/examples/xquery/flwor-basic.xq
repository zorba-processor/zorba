for $b in doc("../data/books.xml")//book
let $price := xs:decimal($b/price)
where $price < 20
order by $price descending
return <cheap price="{$price}">{ $b/title/string() }</cheap>
