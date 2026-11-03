(: W3C Serialization 3.0: an atomic value is serialized through its string value;
   the element content and attribute too, whatever its magnitude. :)
let $m := xs:unsignedLong("18446744073709551615")
return (<a b="{$m}">{$m}</a>, $m)
