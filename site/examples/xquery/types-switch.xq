for $code in ("es", "en", "fr")
return switch ($code)
  case "es" return "Spanish"
  case "en" return "English"
  default return concat("unknown: ", $code)
