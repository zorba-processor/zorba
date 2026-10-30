for $b at $pos in doc("../data/books.xml")//book
order by xs:integer($b/@year) descending
count $rank
return <book rank="{$rank}" position="{$pos}" year="{$b/@year}">{ string($b/title) }</book>
