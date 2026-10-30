copy $o := { "a": 1, "b": [ 1 ] }
modify (
  replace value of json $o("a") with 2,
  append json 2 into $o("b")
)
return $o
