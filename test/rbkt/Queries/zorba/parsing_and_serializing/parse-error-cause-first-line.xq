(: An error on the first line of a fragment is at the same column as in a
   document. :)
declare namespace zerr = "http://zorba.io/errors";

for $parse in (fn:parse-xml#1, fn:parse-xml-fragment#1)
return
  try { $parse("<b></c>") }
  catch * {
    string-join((string($zerr:data-line-number),
                 string($zerr:data-column-number)), " ")
  }
