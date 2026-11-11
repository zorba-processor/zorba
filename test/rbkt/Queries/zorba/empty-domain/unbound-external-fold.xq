(: A for clause whose domain is an unbound external variable of at most one item, its variable unreferenced: the domain is evaluated. :)
declare variable $ext as xs:integer? external;

for $a in $ext
return 1
