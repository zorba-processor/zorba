(: An xs:dayTimeDuration is a subtype of xs:duration, so eq raises no error
   and the join uses an index on the xs:duration keys. A duration of zero
   months equals one of zero seconds. :)
let $doc := <d><t>PT1H</t><t>P1M</t><t>PT60M</t><t>P0M</t></d>
for $k in (xs:dayTimeDuration("PT1H"), xs:dayTimeDuration("PT2H"),
           xs:dayTimeDuration("PT0S"))
let $m := for $t in $doc/t where xs:duration($t) eq $k return $t
return fn:count($m)
