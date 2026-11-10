(: Control: the else branch is the empty sequence, so the condition becomes a where clause. :)
for $x in (1, 2, 3)
return if ($x eq 3) then $x else ()
