### Fixed

- Node ids whose last byte holds several short components (for example `1.2.0.0.0.1`) lost their trailing components when decoded, so repeated insertion of nodes at the same position by XQuery Update produced invalid ids after a few rounds. The stored id format is unchanged.
