### Fixed

- Running out of memory while the types of revalidation (`declare
  revalidation`) or of `schema:validate-in-place` are applied, after the
  updates themselves, is now the memory error, with every update of the
  statement undone. It ended the process (`ZORBA_FATAL`).
- When a statement failed after its revalidation was applied, the default
  attributes that revalidation had added stayed; they are now removed with
  the rest of the statement.
- Undoing the typed value that revalidation gave a simple-content element
  restores the element's text as it was, instead of the typed value's
  canonical form.
