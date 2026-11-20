(: The nodes at the beginning of a queue may come in any order and more than once. :)
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace q = "http://example.org/queue-delete" at "queue-delete.xqdata";

ddl:create($q:queue, (<a/>, <b/>, <c/>, <d/>));
let $nodes := dml:collection($q:queue)
return dml:delete(($nodes[3], $nodes[1], $nodes[2], $nodes[1]));
<queue>{dml:collection($q:queue)}</queue>
