(: The positions and messages fn:parse-xml reports for a document, which the
   fragment tests of this directory match. :)
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:error($input as xs:string) as xs:string
{
  try { "no error: " || serialize(fn:parse-xml($input)) }
  catch * {
    string-join((
      string($zerr:data-line-number),
      string($zerr:data-column-number),
      substring-after($err:description, "loader parsing error: ")
    ), " ")
  }
};

for $input in ("<b></c>",
               "<a>&#10;<b></c>",
               '<?xml version="1.0" encoding="UTF-8"?><b></c>',
               "<a>",
               "<a>&#10;<b>")
return local:error($input)
