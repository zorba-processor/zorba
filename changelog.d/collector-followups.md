### Fixed

- Free the state of a closed query when a cycle through it passes through
  the applied argument of a partially applied function item (e.g. a prolog
  variable holding `$call(function($y) { $y + 1 }, ?)`); before, the cycle
  collector did not follow those arguments and the state leaked.
- Keep the cycle collector usable when a walk of a closed query's graph
  fails (e.g. out of memory): the graph is kept and the next collection
  walks it again; before, the exception escaped from reference releases.
