let $o := { "a": 1, "b": [ 10, 20, 30 ], "c": { "d": true } }
return {
  "keys": [ keys($o) ],
  "size": size($o.b),
  "members": [ members($o.b) ],
  "is-object": $o.c instance of object,
  "sum": sum($o.b[])
}
