[
for $v in (1, "one", true, null, [ 1 ], { "one": 1 })
return typeswitch ($v)
  case object  return "object"
  case array   return "array"
  case null    return "null"
  case boolean return "boolean"
  case string  return "string"
  case integer return "integer"
  default      return "other"
]
