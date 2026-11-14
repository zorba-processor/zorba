(: Node ids are at most 255 bytes, which caps how deep a tree can be: about a
   thousand levels for a chain of single children. Constructing a deeper tree
   exceeds an implementation limit (err:XPDY0130), whose message names the
   nesting depth reached, at the constructor that exceeds it. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:nest($n as xs:integer)
{
  if ($n eq 0)
  then "x"
  else <a>{ local:nest($n - 1) }</a>
};

<result>
  <shallow>{ count(local:nest(1000)/descendant-or-self::a) }</shallow>
  <deep>{
    try { count(local:nest(1100)/descendant-or-self::a) }
    catch * {
      string-join((string($err:code), $err:description,
                   string($err:line-number)), " | ")
    }
  }</deep>
</result>
