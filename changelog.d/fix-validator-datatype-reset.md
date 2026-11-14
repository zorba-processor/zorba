### Fixed

- Lax validation keeps the whitespace of text that follows, in a mixed
  element, a child whose `xsi:type` names a simple type its element-only
  declaration does not derive from. Before, that text was collapsed as if it
  had the child's simple type.
