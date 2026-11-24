(: fn:boolean stops at a first item that is a node, at every level: a later item that raises is not evaluated. :)
declare variable $zero as xs:integer external := 0;

boolean((<a/>, <b>{6 idiv $zero}</b>))
