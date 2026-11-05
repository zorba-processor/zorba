### Added

- The `Benchmarks` workflow compares two commits on one benchmark suite of
  `bench/` when it is dispatched with `suite`, `base` and `head`: both are
  built in Release in one job and each query runs alternately on both, with
  a timeout per run. The job summary has the median and minimum wall time and
  the peak RSS of each build, the ratio head/base, and every failed, timed
  out or not run query. Without `suite` the workflow measures XMark and
  JSONiq as before.
