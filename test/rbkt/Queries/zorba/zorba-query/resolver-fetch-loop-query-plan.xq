(: In a query loaded with zq:load-from-query-plan#3, fetch:content in a loop calls the resolver function item once per iteration. :)
import module namespace zq = 'http://zorba.io/modules/zorba-query';
import module namespace r = 'http://zorba.io/modules/random';
declare namespace an = 'http://zorba.io/annotations';
declare namespace resolver = 'http://zorba.io/modules/zorba-query/url-resolver';

declare %an:nondeterministic function resolver:url-resolver($uri as xs:string, $entity as xs:string) {
  if ($uri = 'http://zorba-tests.example.org/resolver-fetch')
  then r:uuid()
  else ()
};

variable $queryID := zq:prepare-main-module(
  "import module namespace fetch = 'http://zorba.io/modules/fetch';
   count(distinct-values(
     for $i in 1 to 3
     return fetch:content-binary('http://zorba-tests.example.org/resolver-fetch')))",
  resolver:url-resolver#2, ());
variable $plan := zq:query-plan($queryID);
variable $queryID2 := zq:load-from-query-plan($plan, resolver:url-resolver#2, ());
zq:evaluate($queryID2)
