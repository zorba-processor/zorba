import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: xsi:nil must be an xs:boolean: other values make validation fail :)
for $v in ("yes", "TRUE", "", "2", "true false")
return
  try { nilled(validate { <t:n xsi:nil="{$v}"/> }) }
  catch * { local-name-from-QName($err:code) },
for $v in ("true", " 1 ", "false", "0")
return nilled(validate { <t:n xsi:nil="{$v}"/> }),
(: lax validation checks a declared element, not one without a declaration :)
try { nilled((validate lax { <u><t:n xsi:nil="yes"/></u> })/t:n) }
catch * { local-name-from-QName($err:code) },
nilled(validate lax { <u xsi:nil="yes"/> })
