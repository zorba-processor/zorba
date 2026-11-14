declare function local:nest($n as xs:integer)
{
  if ($n eq 0)
  then "x"
  else <a>{ local:nest($n - 1) }</a>
};
(count(local:nest(3)/descendant-or-self::a), count(<a><a><a/></a></a>/descendant-or-self::a), serialize(local:nest(3)))
