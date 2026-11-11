(: Control: the external variable is bound, so neither domain can raise and the FLWOR is the empty sequence. :)
declare variable $ext external;

for $a in $ext
for $b in ()
return $a
