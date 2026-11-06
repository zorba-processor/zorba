let $arr := [ 1, 2 ]
let $v := string-join(for $i in 1 to 300000 return "abcdefghij", "")
return $arr($v)
