for $item in (42, "text", 2.5e0, xs:date("2026-10-29"),
              <e/>, text { "t" }, { "k": 1 }, [ 1, 2 ], upper-case#1)
return typeswitch ($item)
  case xs:integer   return "xs:integer (atomic)"
  case xs:string    return "xs:string (atomic)"
  case xs:double    return "xs:double (atomic)"
  case xs:date      return "xs:date (atomic)"
  case element()    return "element (node)"
  case text()       return "text (node)"
  case object()     return "object (JSON)"
  case array()      return "array (JSON)"
  case function(*)  return "function item"
  default           return "something else"
