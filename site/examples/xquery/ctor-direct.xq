let $books := doc("../data/books.xml")//book
return
  <ul class="books" count="{count($books)}">{
    for $b in $books
    return <li lang="{$b/@lang}">{ string($b/title) } <em>{ string($b/@year) }</em></li>
  }</ul>
