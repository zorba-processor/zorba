import module namespace b = "http://zorba.io/bench/index-probe" at "index_probe.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";

import module namespace iddl =
  "http://zorba.io/modules/store/static/indexes/ddl";

declare variable $n as xs:integer external := 10000;
declare variable $calls as xs:integer external := 200000;

ddl:create($b:items);
iddl:create($b:idx);
b:load($n);

count(for $j in 1 to $calls
      where b:lookup(string(1 + $j mod $n)) ne ""
      return $j)
