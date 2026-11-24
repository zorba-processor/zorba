(: fn:boolean over a node argument whose construction raises: the error is kept. :)
declare variable $zero as xs:integer external := 0;

boolean(<a>{6 idiv $zero}</a>)
