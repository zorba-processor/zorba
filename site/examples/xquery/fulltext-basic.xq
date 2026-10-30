let $docs := (
  <p id="1">The quick brown fox jumps over the lazy dog.</p>,
  <p id="2">Foxes were running through the fields.</p>,
  <p id="3">A slow brown bear sleeps all winter.</p>)
return (
  <plain>{ $docs[. contains text "fox"]/@id/string() }</plain>,
  <stemmed>{ $docs[. contains text "fox" using stemming]/@id/string() }</stemmed>,
  <any-word>{ $docs[. contains text "bear fox" any word]/@id/string() }</any-word>
)
