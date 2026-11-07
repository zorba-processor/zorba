### Fixed

- The modify clause of a copy modify expression raises `XUDY0014` for every
  update whose target is not a node created by its copy clause. Collection
  updates (`ddl:create`, `ddl:delete`, `dml:insert-*`, `dml:delete*`,
  `dml:edit`, `dml:truncate`), `doc:put`, `doc:remove`, the creation, deletion
  and refresh of an index and the activation of an integrity constraint no
  longer change the store from there. Before, only node targets were checked,
  and a node target outside the copy could pass when another target was
  checked first.
- `fn:put` in the modify clause raises `XUDY0037` (XQuery Update Facility 3.0)
  instead of storing the document.
