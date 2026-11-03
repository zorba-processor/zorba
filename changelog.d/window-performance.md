### Changed

- A sliding window clause reads its input as it goes and releases the items that no
  open window can reach, as a tumbling window clause already did, in a query where no
  side effect can run: a sequential expression or function, a dynamic function call, a
  built-in function that takes a function, or eval keep the input of sliding windows in
  memory, as before. An error in the input now surfaces when the clause reaches it, after
  any windows returned before it.
- A window clause releases the items before and between windows while it reads them,
  and keeps no items for open windows if the query refers to neither the window
  variable nor the items of its start and end variables.

### Fixed

- A sliding window clause no longer evaluates its end condition once per open window
  when the condition reads no start variable and is made of repeatable built-in functions:
  one evaluation per item, with the same results. Other conditions, such as those that
  call a user function or make a node, are still evaluated once per window.

### Breaking changes

- Query plans saved by an earlier version are rejected with ZCSE0012: the plan of a
  window clause changed.
