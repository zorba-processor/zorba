(: The smallest 64-bit integer can be formatted (F&O 3.0 4.6.1). :)
let $min := xs:integer("-9223372036854775807") - 1
return (format-integer($min, "1"),
        format-integer($min, "1;o"),
        format-integer(-21, "1;o"))
