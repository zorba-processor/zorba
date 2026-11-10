(: Inside a prepared query, each call of fetch:content calls the resolver function item: a resolver that returns a new value per call gives two values for two calls with the same URI. :)
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
   declare variable $u := 'http://zorba-tests.example.org/resolver-fetch';
   count(distinct-values((fetch:content($u), fetch:content($u))))",
  resolver:url-resolver#2, ());
zq:evaluate($queryID)
