(: Fractional seconds beyond the supported 6 digits are truncated; they must
   not overflow into the seconds field (XSD 1.1 Part 2, 3.3.7; 5.4). :)
let $dt := xs:dateTime("1999-12-31T23:59:59.999999999Z")
return (string($dt),
        string(seconds-from-dateTime($dt)),
        string(xs:time("23:59:59.9999999")),
        string(xs:dateTime("2000-01-01T00:00:00.0000005")),
        string(xs:dateTime("2000-01-01T00:00:00.000003")))
