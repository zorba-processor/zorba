(: delete-nodes on a queue collection accepts the nodes at its beginning. :)
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace q = "http://example.org/queue-delete" at "queue-delete.xqdata";

ddl:create($q:queue, (<a/>, <b/>, <c/>, <d/>));
dml:delete(subsequence(dml:collection($q:queue), 1, 2));
<queue>{dml:collection($q:queue)}</queue>
