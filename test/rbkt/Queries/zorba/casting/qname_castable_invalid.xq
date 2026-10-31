(: castable as xs:QName returns false for an invalid lexical form instead of
   raising an error (XQuery 3.0 3.13.3). :)
(("") castable as xs:QName,
 ("1a") castable as xs:QName,
 ("a:b:c") castable as xs:QName,
 ("a") castable as xs:QName)
