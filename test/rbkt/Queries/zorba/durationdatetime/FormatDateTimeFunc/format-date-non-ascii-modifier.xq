declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: A first presentation modifier that is a non-ASCII, non-digit character
   is an unsupported numbering sequence (format token 1); it raised the
   internal error ZXQP0003 (invalid UTF-8). :)
format-date(xs:date("2003-09-07"), "[Y&#xE40;]"),
try { format-date(xs:date("2003-09-07"), "[Y&#xE40;&#xE50;&#xE10;&#xE51;]") }
catch * { local-name-from-QName($err:code) }
