(: The line and column of a parser error are its position in the string, as
   for a document; the message names no element the string does not have. :)
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:error($input as xs:string) as xs:string
{
  try { "no error: " || serialize(fn:parse-xml-fragment($input)) }
  catch * {
    string-join((
      string($zerr:data-line-number),
      string($zerr:data-column-number),
      substring-after($err:description, "loader parsing error: ")
    ), " ")
  }
};

for $input in ("<b></c>",
               "<a/>&#10;<b></c>",
               '<?xml encoding="UTF-8"?><b></c>',
               '<?xml encoding="UTF-8"?>&#10;<b></c>',
               "<a>",
               "<a/>&#10;<b>",
               "<a/></b>")
return local:error($input)
