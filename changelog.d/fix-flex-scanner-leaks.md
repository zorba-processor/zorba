### Fixed

- The XQuery and JSONiq scanners no longer lose memory when an allocation
  fails while they create or grow a buffer or their start-condition stack
  (flex's skeleton dropped the old block or the buffer struct). The fix is a
  patch applied to the generated scanners, so it survives regeneration.
