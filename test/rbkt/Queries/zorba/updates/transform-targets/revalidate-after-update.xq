(: The modify clause updates the copy, then revalidates a node that the copy
   clause did not create. :)
import module namespace schema = "http://zorba.io/modules/schema";
declare revalidation lax;
let $outside := <o/>
return
  copy $c := <r/>
  modify (insert node <x/> into $c, schema:validate-in-place($outside))
  return $c
