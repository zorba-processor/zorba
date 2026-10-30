(: A single item and a one-item sequence are the same thing. :)
(
  deep-equal(42, (42)),
  count(42),
  (42)[1] + 0,
  count(()),
  empty(())
)
