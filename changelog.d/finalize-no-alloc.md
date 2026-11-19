### Fixed

- Detaching the nodes that updates removed (deleted, replaced, and text
  nodes merged together), done after the updates are committed, no longer
  aborts the process when no memory is left: what it needs is made while the
  updates are applied, where a failure still undoes them.
