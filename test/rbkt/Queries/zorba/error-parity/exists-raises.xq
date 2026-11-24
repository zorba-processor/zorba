(: fn:exists over an argument whose cardinality is known and which raises: the error is kept. :)
declare variable $zero as xs:integer external := 0;

exists(if (6 idiv $zero eq 1) then () else ())
