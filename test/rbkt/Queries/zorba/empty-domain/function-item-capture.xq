(: A one-item for domain that is a function item capturing a let variable whose binding raises: creating the function item evaluates the captured variable, so the error is raised. :)
declare variable $zero as xs:integer external := 0;

let $y := 1 idiv $zero
for $f in function() { $y }
return 1
