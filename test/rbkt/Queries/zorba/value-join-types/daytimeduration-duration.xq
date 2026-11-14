(: xs:duration is not a subtype of the inner xs:dayTimeDuration, but one is a
   subtype of the other, so eq raises no error and the join uses an index with
   normalized keys. A duration of zero months equals one of zero seconds. :)
let $doc := <d><t>PT1H</t><t>PT60M</t><t>PT0S</t></d>
for $k in (xs:duration("PT1H"), xs:duration("P1M"), xs:duration("P0M"))
let $m := for $t in $doc/t where xs:dayTimeDuration($t) eq $k return $t
return fn:count($m)
