for $b at $pos in doc("../data/books.xml")//book
order by xs:integer($b/@year) descending
count $rank
return concat($rank, ". ", $b/title, " (", $b/@year, "), document position ", $pos)
