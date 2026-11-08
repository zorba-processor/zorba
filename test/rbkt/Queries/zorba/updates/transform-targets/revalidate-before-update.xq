(: The modify clause revalidates a node that the copy clause did not create,
   then updates the copy. :)
import module namespace schema = "http://zorba.io/modules/schema";
declare revalidation lax;
let $outside := <o/>
return
  copy $c := <r/>
  modify (schema:validate-in-place($outside), insert node <x/> into $c)
  return $c
