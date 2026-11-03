declare variable $s external := "[1]";
let $a := parse-json($s)
return ($a[], $a[])
