### Changed

- A general `=` with a global variable on one side (`declare variable`), or a
  local variable of a sequential block, evaluated again for the same value of
  the variable, hashes the items of the variable once and looks up each item
  of the other side, as it already did for a `let` variable. A new value
  assigned to the variable is hashed anew. The answer and the errors stay
  those of comparing every pair.
