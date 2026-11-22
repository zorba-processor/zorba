(: An end tag at the top level of a fragment is not well balanced, whatever its
   name; the error is at the end of that tag, not where the input ends. An
   element left open is reported where the input ends, counted in the
   characters that the declared encoding makes of the input. :)
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

for $input in ("<a/></zorba-fragment-wrapper>",
               "<a/></zorba-fragment-wrapper>&#10;",
               "<a/></zorba-fragment-wrapper><!--c-->",
               "</zorba-fragment-wrapper><b/>",
               "<a/>&#10;</zorba-fragment-wrapper>t",
               '<?xml encoding="ISO-8859-1"?>é<b></c>',
               '<?xml encoding="ISO-8859-1"?>é<b>',
               '<?xml encoding="UTF-8"?>é<b>')
return local:error($input)
