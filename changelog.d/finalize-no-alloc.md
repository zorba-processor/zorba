### Fixed

- Detaching the nodes that updates removed (deleted, replaced, and text
  nodes merged together), done after the updates are committed, no longer
  aborts the process when no memory is left: what it needs is made while the
  updates are applied, where a failure still undoes them.
- Running out of memory while the security policy resolves a file path is a
  memory error, not an access denied for a malformed path (`FOFL9999`), so the
  document cache no longer refuses that file for good.
