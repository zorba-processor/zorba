(: F&O 3.0 19.1.2 and 19.1.3: casting from a type derived from xs:integer. The
   value is preserved when the target can hold it; 19.2 FORG0001 outside the range
   of a derived target. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
let $m := xs:unsignedLong("18446744073709551615")
let $b := xs:unsignedLong("9223372036854775808")
return (
  string($m), string($b),
  xs:string($m), xs:untypedAtomic($m) instance of xs:untypedAtomic,
  xs:decimal($m), xs:decimal($b),
  xs:double($m) eq 1.8446744073709551616E19, xs:float($m) eq xs:float("1.8446744073709551616E19"),
  xs:boolean($m),
  try { xs:long($m) } catch * { local-name-from-QName($err:code) },
  try { xs:unsignedInt($m) } catch * { local-name-from-QName($err:code) },
  concat($m, "|", $b),
  string(<a b="{$m}">{$b}</a>/@b), string(<a b="{$m}">{$b}</a>)
)
