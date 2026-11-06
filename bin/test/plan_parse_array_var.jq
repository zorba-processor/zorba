declare variable $s external := "[1]";
let $a := parse-json($s, { "zorba-streaming-members" : true })
return ($a[], $a[])
