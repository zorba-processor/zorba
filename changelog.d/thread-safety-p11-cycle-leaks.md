### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, two leaks of the state
  of a closed query that its function items keep alive through a cycle (a
  function item stored in a prolog variable). A function item released, or
  destroyed, on one thread while another thread closed its query could drop
  the last reference from outside the cycle after the close had counted it;
  the release now collects the queries closed meanwhile. A collection whose
  walk saw a reference count change released nothing and was not retried,
  also when the count belonged to a JSON item or temporary sequence that the
  cycle does not depend on; it now checks only the counts of the nodes it
  found unreachable, and walks again if one of them changed.
