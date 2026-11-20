(: The occurrence indicator of a collection declaration is checked when the
   collection is read: dml:collection raises err:XPTY0004 when the number of
   nodes in the collection does not match it. Inserting is not checked. :)
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace o = "http://example.org/occurrence" at "occurrence.xqdata";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:error($code, $description, $line) as xs:string
{
  string-join((string($code), $description, string($line)), " | ")
};

declare function local:read() as element(read)
{
  <read>{
    <one>{
      try { let $c := dml:collection(xs:QName("o:one")) return count($c) }
      catch * { local:error($err:code, $err:description, $err:line-number) }
    }</one>,
    <optional>{
      try { let $c := dml:collection(xs:QName("o:optional")) return count($c) }
      catch * { local:error($err:code, $err:description, $err:line-number) }
    }</optional>,
    <some>{
      try { let $c := dml:collection(xs:QName("o:some")) return count($c) }
      catch * { local:error($err:code, $err:description, $err:line-number) }
    }</some>,
    <any>{
      try { let $c := dml:collection(xs:QName("o:any")) return count($c) }
      catch * { local:error($err:code, $err:description, $err:line-number) }
    }</any>,
    <count>{
      try { count(dml:collection(xs:QName("o:one"))) }
      catch * { local:error($err:code, $err:description, $err:line-number) }
    }</count>,
    <skip>{
      try { count(dml:collection(xs:QName("o:one"), 1)) }
      catch * { local:error($err:code, $err:description, $err:line-number) }
    }</skip>
  }</read>
};

declare %an:sequential function local:insert()
{
  dml:insert-last(xs:QName("o:one"), <a/>);
  dml:insert-last(xs:QName("o:optional"), <a/>);
  dml:insert-last(xs:QName("o:some"), <a/>);
  dml:insert-last(xs:QName("o:any"), <a/>);
};

ddl:create(xs:QName("o:one"));
ddl:create(xs:QName("o:optional"));
ddl:create(xs:QName("o:some"));
ddl:create(xs:QName("o:any"));

<result>{
  local:read(),
  { local:insert(); local:read() },
  { local:insert(); local:read() }
}</result>
