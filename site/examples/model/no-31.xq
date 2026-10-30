(: Compile a few queries at run time and report what Zorba accepts. :)
import module namespace zq = "http://zorba.io/modules/zorba-query";
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %an:sequential function local:compile($query as xs:string) as xs:string
{
  try {
    { variable $key := zq:prepare-main-module($query); "compiles" }
  }
  catch * { "error " || local-name-from-QName($err:code) }
};

for $query in ('xquery version "3.0"; 1 + 1',
               'xquery version "3.1"; 1 + 1',
               'map { "a": 1 }',
               '(1, 2, 3) => count()',
               '{ "a": 1 }("a")')
return <compile query="{$query}">{ local:compile($query) }</compile>
