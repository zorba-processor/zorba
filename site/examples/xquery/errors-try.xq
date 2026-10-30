declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare namespace app = "http://example.org/app";

(
  <cast>{
    try { xs:integer("abc") }
    catch err:FORG0001 { "not a number: " || $err:code }
  }</cast>,
  <division>{
    try { 1 div 0 }
    catch * { "caught " || local-name-from-QName($err:code) }
  }</division>,
  <custom>{
    try { error(QName("http://example.org/app", "app:E42"), "stock is negative") }
    catch app:E42 { $err:description }
  }</custom>
)
