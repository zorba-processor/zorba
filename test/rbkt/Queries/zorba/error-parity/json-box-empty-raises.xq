(: An object value of the empty sequence type that raises: the error is kept. :)
declare variable $zero as xs:integer external := 0;

{ "a" : if (6 idiv $zero eq 1) then () else () }
