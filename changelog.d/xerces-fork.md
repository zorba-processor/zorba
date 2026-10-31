### Fixed

- Fix schema import rejecting valid XSD 1.0 schemas that restrict a union-typed element to a restriction of the union (`XQST0059`, Xerces-C bug): the Linux CI, builder and `zorba` runtime images build Xerces-C from the patched fork [zorba-processor/xerces-c](https://github.com/zorba-processor/xerces-c) (`v3.3.0-zorba.2`), and the 161 QT3 test-cases importing `unionListDefined.xsd` now run; macOS and other stock Xerces-C builds still have the bug.
