let $books := doc("../data/books.xml")//book
return
  <ul class="books" count="{count($books)}">{
    for $b in $books
    return <li lang="{$b/@lang}" year="{$b/@year}">{ string($b/title) }</li>
  }</ul>
