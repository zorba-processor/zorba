(: The modify clause renames the 50 children of the copy and one node that the
   copy clause did not create. The error must not depend on the order in which
   the targets are checked. :)
let $outside := <o/>
return
  copy $c := <r>{for $i in 1 to 50 return <x/>}</r>
  modify (for $x in $c/x return rename node $x as "y",
          rename node $outside as "z")
  return $c
