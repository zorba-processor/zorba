(: Value join between xs:anyURI and xs:string / xs:untypedAtomic operands:
   the operand types differ, so the index join rule uses normalized keys. :)
let $doc := <r><s v="b"/><s v="c"/><s v="a"/><s v="b"/></r>
for $u in (xs:anyURI("a"), xs:anyURI("b"), xs:anyURI("d"))
let $m :=
  for $s in $doc/s
  where $u eq xs:string($s/@v)
  return $s
return <u v="{$u}">{count($m)}</u>
