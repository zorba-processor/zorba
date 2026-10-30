for $b in doc("../data/books.xml")//book
let $price := xs:decimal($b/price)
group by $author := string($b/author)
order by count($b) descending, $author
return <author name="{$author}" books="{count($b)}" total="{sum($price)}"/>
