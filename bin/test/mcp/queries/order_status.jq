(: Returns the order with the given id (from the XML orders) together with
 : its payment status (from the JSON payments). :)

(:
 : Copyright 2006-2016 zorba.io
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
:)

import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $order external;

let $payments :=
  for $d in dir:documents($dir:root, "json")
  let $c := $d("content")
  where exists($c)
  return if ($c instance of object()) then $c else jn:members($c)
for $doc in dir:documents($dir:root, "xml")
for $o in $doc("content")/orders/order[@id eq $order]
return {
  "order" : $order,
  "customer" : string($o/@customer),
  "total" : xs:integer($o/total),
  "status" : [ for $p in $payments where $p("order") eq $order return $p("status") ]
}
