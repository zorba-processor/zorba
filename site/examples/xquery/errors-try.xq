declare namespace app = "http://example.org/app";

(
  try { xs:integer("abc") }
  catch err:FORG0001 { "not a number: " || $err:code },

  try { 1 div 0 }
  catch * { "caught " || local-name-from-QName($err:code) },

  try {
    error(QName("http://example.org/app", "app:E42"), "stock is negative")
  }
  catch app:* { $err:description }
)
