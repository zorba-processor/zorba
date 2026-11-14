(: Control: a one-item for domain that raises is referenced only in the condition of the return clause; the if becomes a where clause, whose expression is evaluated for every tuple, so the for variable is folded into it. :)
declare variable $two as xs:integer external := 2;

for $x in (1, $two)
for $a in 6 idiv $x
return if ($a eq 6) then $x else ()
