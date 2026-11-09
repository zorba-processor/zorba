(: Inside a prepared query, fn:doc is stable: the same URI gives the same document, also when the resolver function item returns a new value per call. :)
import module namespace zq = 'http://zorba.io/modules/zorba-query';
import module namespace r = 'http://zorba.io/modules/random';
declare namespace an = 'http://zorba.io/annotations';
declare namespace resolver = 'http://zorba.io/modules/zorba-query/url-resolver';

declare %an:nondeterministic function resolver:url-resolver($uri as xs:string, $entity as xs:string) {
  if ($uri = 'http://zorba-tests.example.org/resolver-doc')
  then <d>{ r:uuid() }</d>
  else ()
};

variable $queryID := zq:prepare-main-module(
  "declare variable $u := 'http://zorba-tests.example.org/resolver-doc';
   (doc($u) is doc($u),
    count(distinct-values(for $i in 1 to 3 return string(doc($u)))))",
  resolver:url-resolver#2, ());
zq:evaluate($queryID)
