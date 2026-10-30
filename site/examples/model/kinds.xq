let $items := (42, "text", 2.5e0, xs:date("2026-10-29"),
               <e/>, text { "t" }, { "k": 1 }, [ 1, 2 ], upper-case#1)
let $kinds :=
  for $item in $items
  return typeswitch ($item)
    case xs:integer   return "xs:integer   atomic value"
    case xs:string    return "xs:string    atomic value"
    case xs:double    return "xs:double    atomic value"
    case xs:date      return "xs:date      atomic value"
    case element()    return "element()    node"
    case text()       return "text()       node"
    case object()     return "object()     JSON item"
    case array()      return "array()      JSON item"
    case function(*)  return "function(*)  function item"
    default           return "something else"
return string-join($kinds, "&#10;")
