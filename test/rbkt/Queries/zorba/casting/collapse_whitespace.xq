(: Types derived from xs:token collapse whitespace before validation
   (XSD 1.1 Part 2, 3.4.3-3.4.9; F&O 3.0 19.1.5). :)
for $v in ((" true ") cast as xs:language,
           (" en-US&#xA;") cast as xs:language,
           (" a1 ") cast as xs:NMTOKEN,
           (" ab ") cast as xs:Name,
           (" abc ") cast as xs:NCName,
           ("&#x9;abc ") cast as xs:ID,
           (" abc ") cast as xs:IDREF,
           (" abc ") cast as xs:ENTITY)
return concat("[", $v, "]")
