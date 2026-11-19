(: Deleting every node of a queue empties it. :)
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace q = "http://example.org/queue-delete" at "queue-delete.xqdata";

ddl:create($q:queue, (<a/>, <b/>));
dml:delete(dml:collection($q:queue));
<queue>{dml:collection($q:queue)}</queue>
