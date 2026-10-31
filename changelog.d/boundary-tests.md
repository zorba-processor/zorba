### Added

- Add generated boundary-value matrices and property tests to the differential testing workflow (`test/differential/boundary.py`, `property.py`; `set=rbkt|boundary|property`): about 38,000 deterministic boundary cases per atomic type (operators, comparisons, casts, string, numeric, date/time and aggregate functions) run on Zorba and Saxon-HE, and about 5,900 seeded random property cases (serialization and cast round trips, inverses, idempotence, sort stability, Python oracles) run on Zorba alone; results are informational.
