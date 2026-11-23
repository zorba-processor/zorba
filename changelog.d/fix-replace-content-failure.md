### Fixed

- Fix a failed `replace value of node` on an element leaving it half-updated:
  when memory ran out after the old children were taken out, the element kept
  the new text and lost its old children, which were never freed. The element
  now gets its old children back.
