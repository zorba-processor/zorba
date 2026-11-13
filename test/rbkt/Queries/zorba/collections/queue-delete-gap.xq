(: delete-nodes on a queue collection rejects a node that is not at its beginning. :)
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace q = "http://example.org/queue-delete" at "queue-delete.xqdata";

ddl:create($q:queue, (<a/>, <b/>, <c/>, <d/>));
let $nodes := dml:collection($q:queue)
return dml:delete(($nodes[1], $nodes[3]));
<queue>{dml:collection($q:queue)}</queue>
