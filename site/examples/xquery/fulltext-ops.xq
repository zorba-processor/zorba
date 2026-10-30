let $docs := (
  <p id="1">The quick brown fox jumps over the lazy dog.</p>,
  <p id="2">Foxes were running through the fields.</p>,
  <p id="3">A slow brown bear sleeps all winter.</p>)
return (
  <and>{ $docs[. contains text "brown" ftand "dog"]/@id/string() }</and>,
  <not>{ $docs[. contains text "brown" ftand ftnot "bear"]/@id/string() }</not>,
  <window>{ $docs[. contains text "quick" ftand "fox" window 3 words]/@id/string() }</window>,
  <wildcard>{ $docs[. contains text "run.*" using wildcards]/@id/string() }</wildcard>
)
