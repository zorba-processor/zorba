### Fixed

- `math:percentile` of the math module (`http://zorba.io/modules/math`)
  returns the exact percentile: the numbers are sorted and the result is read
  at rank (n - 1) * k, interpolating linearly between the two closest ranks
  (Excel PERCENTILE.INC, numpy and SQL `PERCENTILE_CONT` "linear"). Before,
  it returned min + (max - min) * k. An empty sequence raises
  `math:INVALID_ARGUMENT`, and a NaN among the numbers gives NaN.
- An `import` placed after a variable, function, option or other declaration
  of the prolog raises XPST0003 with the message "imports must come before
  variable, function, option and other declarations", located at the import.
  Before, the error was a generic syntax error.

### Changed

- `zorba --help` and the command-line documentation say how to write JSON
  Lines (`-z method=json -z item-separator=<newline> --trailing-nl`) and the
  precedence of serialization parameters: prolog, then `-z`, then `--indent`,
  `--serialize-html` and `--serialize-text`.
