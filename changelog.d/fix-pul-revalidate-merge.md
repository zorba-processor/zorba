### Fixed

- Fix the modify clause of a copy/modify expression accepting
  `schema:validate-in-place` on a node that the copy clause did not create
  when it followed another update of the same modify clause; it now raises
  `err:XUDY0014` in either order.
