(: A one-item for domain that references a let variable whose binding raises, with the for variable not referenced: the domain is evaluated. :)
declare variable $zero as xs:integer external := 0;
declare variable $flag as xs:boolean external := false();

let $l := 6 idiv $zero
return (for $a in $l return 1, if ($flag) then $l else ())
