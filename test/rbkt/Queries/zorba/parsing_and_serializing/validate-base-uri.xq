(: Validation gives every element of the copy the base URI it had: inherited
   from the document, set by xml:base, and inherited from that. :)
let $d := doc("validate-base-uri.xml")
let $v := validate lax { $d }
return (
  deep-equal(for $e in $d//* return base-uri($e),
             for $e in $v//* return base-uri($e)),
  count($v//*)
)
