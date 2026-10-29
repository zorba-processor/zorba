import module namespace xqd = "http://zorba.io/modules/xqdoc";
import schema namespace xqds = "http://www.xqdoc.org/1.0";

declare namespace an = "http://zorba.io/annotations";

(: Directory of this query, used to locate the module under test. :)
declare variable $local:here := fn:resolve-uri(".", fn:static-base-uri());

(: The generation date changes on every run; blank it out. :)
declare %an:sequential function local:without-date($doc)
{
  replace value of node ($doc//xqds:date)[1] with "";
  exit returning $doc;
};

local:without-date(
  validate lax { xqd:xqdoc($local:here || "colldecls.xqlib") })
