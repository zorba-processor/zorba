jsoniq version "1.0";
(: zorba --serialize-text -f -q gen.jq -e n:=N -e form:=lines|array > FILE :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $n as string external;
declare variable $form as string external;
let $lines := for $i in 1 to integer($n) return b:line($i)
return
  if ($form eq "array")
  then "[" || string-join($lines, ",\n") || "]"
  else string-join($lines, "\n")
