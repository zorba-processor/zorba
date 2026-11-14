(: The positions and messages fn:parse-xml reports for a document, which the
   fragment tests of this directory match. For an element left open only the
   position: libxml2 2.9 words that error as extra content at the end. :)
declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:error($input as xs:string, $message as xs:boolean)
  as xs:string
{
  try { "no error: " || serialize(fn:parse-xml($input)) }
  catch * {
    string-join((
      string($zerr:data-line-number),
      string($zerr:data-column-number),
      substring-after($err:description, "loader parsing error: ")[$message]
    ), " ")
  }
};

local:error("<b></c>", true()),
local:error("<a>&#10;<b></c>", true()),
local:error('<?xml version="1.0" encoding="UTF-8"?><b></c>', true()),
local:error("<a>", false()),
local:error("<a>&#10;<b>", false())
