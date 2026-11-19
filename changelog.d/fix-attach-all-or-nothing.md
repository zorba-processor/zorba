### Fixed

- Inserting a node (`insert node`, `replace node`) no longer leaves the
  inserted subtree half moved into its new tree when an allocation fails
  after its node ids are computed: moving types, making room in the parent,
  re-resolving `xml:base` and collecting the subtree now happen before the
  subtree changes, and nothing after that can fail.
- Copying an element no longer crashes when an allocation fails while the
  copy's root element is created.
