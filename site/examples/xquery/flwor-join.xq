let $people := (<person name="William Gibson" born="1948"/>,
                <person name="Ursula K. Le Guin" born="1929"/>)
for $p in $people
for $b allowing empty in doc("../data/books.xml")//book[author = $p/@name]
return <row author="{$p/@name}" title="{$b/title}"/>
