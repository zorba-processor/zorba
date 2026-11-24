(: fn:empty over an argument whose cardinality is known and which raises: the error is kept. :)
declare variable $zero as xs:integer external := 0;

empty(6 idiv $zero)
