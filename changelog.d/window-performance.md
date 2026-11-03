### Changed

- A sliding window clause reads its input as it goes and releases the items that no
  open window can reach, as a tumbling window clause already did. An error in the
  input now surfaces when the clause reaches it, after any windows returned before it.
- A window clause releases the items before and between windows while it reads them,
  and keeps no items for open windows if the query refers to neither the window
  variable nor the items of its start and end variables.

### Fixed

- A sliding window clause no longer evaluates its end condition once per open window
  when the condition reads no start variable and has no call to a user or external
  function: one evaluation per item, with the same results.
